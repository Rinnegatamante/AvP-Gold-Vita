#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fixer.h"

#include "bink.h"

#include <AL/al.h>
#include <AL/alc.h>

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/channel_layout.h"
#include "libavutil/imgutils.h"
#include "bink_decode.h"
#include <SDL.h>
#include "libswscale/swscale.h"


//#define DISABLE_MOVIES
//#define DISABLE_MUSIC
//#define DISABLE_FMVS

extern void FlipBuffers(void);
extern void ClearScreenToBlack(void);
extern int SoundSys_IsOn();
extern void DrawAvpMenuBink(char* buf, int width, int height, int pitch);
extern float PlatVolumeToGain(int volume);


//#define AL_CHECK()		{ int err = alGetError(); if(err!=AL_NO_ERROR) printf("%s:%d ALError %04x\n", __FILE__, __LINE__, err); }
#define AL_CHECK() {}

#define FRAMEQUEUESIZE	4

static BOOL binkInitialized = FALSE;

struct binkMovie
{
	AVFormatContext*	avContext;
	AVPacket*			packet;
	AVFrame*			decodeFrame;

	int					videoStreamIndex;
	const AVCodec*		videoCodec;
	AVCodecContext* 	videoCodecContext;
	AVFrame* 			videoFrame;
	struct SwsContext*	videoScaleContext;
	struct {
		uint8_t *data[4];
		int linesize[4];
	} videoScalePicture;
	uint				videoScaleWidth;
	uint				videoScaleHeight;
	uint				videoScaleFormat;
	float				videoFrameRate;
	
	int					audioStreamIndex;
	const AVCodec*		audioCodec;
	AVCodecContext* 	audioCodecContext;
	AVFrame* 			audioFrame;
	char*				audioTempBuffer;

	BOOL				alInited;
	ALuint				alSource;
	ALuint				alBuffers[FRAMEQUEUESIZE];
	ALuint				alFreeBuffers[FRAMEQUEUESIZE];
	ALuint				alNumFreeBuffers;
	ALuint				alNumChannels;
	ALenum				alFormat;
	ALuint				alSampleRate;

	uint				timeLastUpdate;
	
	BOOL				looping;
	BOOL				isfmv;
};


//-----------------------------------------------------------------------------------------------

int BinkDecodeFrame(struct binkMovie* aMovie);

void BinkRenderMovie(struct binkMovie* aMovie)
{
	if(aMovie && aMovie->videoFrame && aMovie->videoScalePicture.data[0])
	{
		DrawAvpMenuBink(
			aMovie->videoScalePicture.data[0],
			aMovie->videoFrame->width,
			aMovie->videoFrame->height,
			aMovie->videoScalePicture.linesize[0]);
	}
}


void BinkInitMovieStruct(struct binkMovie* aMovie)
{
	memset((void*)aMovie, 0, sizeof(struct binkMovie));
	aMovie->videoStreamIndex = -1;
	aMovie->audioStreamIndex = -1;
}

void BinkReleaseMovie(struct binkMovie* aMovie)
{
	if(aMovie->alInited)
	{
		alSourceStop(aMovie->alSource);
		alDeleteSources(1, &aMovie->alSource);
		alDeleteBuffers(FRAMEQUEUESIZE, aMovie->alBuffers);
		if(aMovie->audioTempBuffer)
			free(aMovie->audioTempBuffer);
	}

	if(aMovie->videoScaleContext)
		av_freep(&aMovie->videoScalePicture.data[0]);
	av_frame_free(&aMovie->decodeFrame);
	av_packet_free(&aMovie->packet);
	avcodec_free_context(&aMovie->videoCodecContext);
	avcodec_free_context(&aMovie->audioCodecContext);

	if(aMovie->avContext)
		avformat_close_input(&aMovie->avContext);
		
	BinkInitMovieStruct(aMovie);
}


int BinkStartMovie(struct binkMovie* aMovie, const char* aFilename, BOOL aLoopFlag, BOOL aFmvFlag, BOOL aMusicFlag)
{
	BinkInitMovieStruct(aMovie);
	aMovie->looping = aLoopFlag;
	aMovie->packet = av_packet_alloc();
	aMovie->decodeFrame = av_frame_alloc();
	if(!aMovie->packet || !aMovie->decodeFrame)
	{
		BinkReleaseMovie(aMovie);
		return 0;
	}
	
	if(aFmvFlag)
	{
		aMovie->videoScaleWidth = 128;
		aMovie->videoScaleHeight = 96;
		aMovie->videoScaleFormat = AV_PIX_FMT_RGB24;
	}
	
	if(avformat_open_input(&aMovie->avContext, aFilename, NULL, NULL) < 0)
	{
		BinkReleaseMovie(aMovie);
		return 0;
	}
	 
	if(!avformat_find_stream_info(aMovie->avContext, NULL) < 0)
	{
		BinkReleaseMovie(aMovie);
		return 0;
	}

	int numStreams = 0;
	for(int i=0; i<aMovie->avContext->nb_streams; i++)
	{
		const AVCodecParameters* parameters = aMovie->avContext->streams[i]->codecpar;
		const AVCodec* codec = avcodec_find_decoder(parameters->codec_id);
		if(codec)
		{
			if((parameters->codec_type==AVMEDIA_TYPE_VIDEO && (aMusicFlag || aMovie->videoStreamIndex>=0)) || (parameters->codec_type==AVMEDIA_TYPE_AUDIO && aMovie->audioStreamIndex>=0))
				continue;
			if(parameters->codec_type!=AVMEDIA_TYPE_VIDEO && parameters->codec_type!=AVMEDIA_TYPE_AUDIO)
				continue;

			AVCodecContext* codec_context = avcodec_alloc_context3(codec);
			if(!codec_context)
			{
				BinkReleaseMovie(aMovie);
				return 0;
			}
			if(avcodec_parameters_to_context(codec_context, parameters) < 0 || avcodec_open2(codec_context, codec, NULL) < 0)
			{
				avcodec_free_context(&codec_context);
				continue;
			}

			if(codec_context->codec_type==AVMEDIA_TYPE_VIDEO && !aMusicFlag)
			{
				aMovie->videoCodec = codec;
				aMovie->videoCodecContext = codec_context;
				aMovie->videoStreamIndex = i;
				aMovie->videoFrame = av_frame_alloc();
				numStreams++;
				
				aMovie->videoFrameRate = (float)aMovie->avContext->streams[i]->avg_frame_rate.num / (float)aMovie->avContext->streams[i]->avg_frame_rate.den;
			}
			if(codec_context->codec_type==AVMEDIA_TYPE_AUDIO)
			{
				aMovie->audioCodec = codec;
				aMovie->audioCodecContext = codec_context;
				aMovie->audioStreamIndex = i;
				aMovie->audioFrame = av_frame_alloc();
				numStreams++;
			}
		}
	}
	
	if(aMovie->videoStreamIndex < 0 && aMovie->audioStreamIndex < 0)
	{
		BinkReleaseMovie(aMovie);
		return 0;
	}
	
	if(!aFmvFlag)
	{
		for(int i=0; i<(FRAMEQUEUESIZE-1) * numStreams; i++)
			BinkDecodeFrame(aMovie);
	}

	aMovie->timeLastUpdate = SDL_GetTicks();
	return 1;
}


static int BinkProcessFrame(struct binkMovie* aMovie, int video)
{
	// decode video frame
	if(video)
	{
		if(aMovie->videoScaleContext==NULL)
		{
			if(aMovie->videoScaleWidth==0)	aMovie->videoScaleWidth  = aMovie->videoFrame->width;
			if(aMovie->videoScaleHeight==0)	aMovie->videoScaleHeight = aMovie->videoFrame->height;
			if(aMovie->videoScaleFormat==0)	aMovie->videoScaleFormat = AV_PIX_FMT_RGB565;
			
			aMovie->videoScaleContext = sws_getContext(
				aMovie->videoFrame->width,	aMovie->videoFrame->height,	aMovie->videoFrame->format, 
				aMovie->videoScaleWidth, 	aMovie->videoScaleHeight,	aMovie->videoScaleFormat,
				SWS_FAST_BILINEAR, NULL, NULL, NULL);

			if(aMovie->videoScaleContext==NULL)
				return 0;
		
			av_image_alloc(aMovie->videoScalePicture.data, aMovie->videoScalePicture.linesize, aMovie->videoScaleWidth, aMovie->videoScaleHeight, aMovie->videoScaleFormat, 1);
		}

		sws_scale(aMovie->videoScaleContext, (const uint8_t* const*)aMovie->videoFrame->data, aMovie->videoFrame->linesize, 0, aMovie->videoFrame->height, aMovie->videoScalePicture.data, aMovie->videoScalePicture.linesize);
	}
	
	// decode audio frame
	else
	{

		if(!SoundSys_IsOn())
			return 0;

		if(!aMovie->alInited)
		{
			alGenSources(1, &aMovie->alSource);
			AL_CHECK();
			
			alGenBuffers(FRAMEQUEUESIZE, aMovie->alBuffers);
			AL_CHECK();
			
			alSource3f(aMovie->alSource, AL_POSITION, 0.0, 0.0, 0.0);
			alSource3f(aMovie->alSource, AL_VELOCITY, 0.0, 0.0, 0.0);
			alSource3f(aMovie->alSource, AL_DIRECTION, 0.0, 0.0, 0.0);
			alSourcef (aMovie->alSource, AL_ROLLOFF_FACTOR, 0.0);
			alSourcei (aMovie->alSource, AL_SOURCE_RELATIVE, AL_TRUE);
			alSourcef (aMovie->alSource, AL_PITCH, 1);
			alSourcef (aMovie->alSource, AL_GAIN, 1.0);
			
			AL_CHECK();

			aMovie->alNumFreeBuffers=FRAMEQUEUESIZE;
			for(int i=0; i<aMovie->alNumFreeBuffers; i++)
				aMovie->alFreeBuffers[i] = aMovie->alBuffers[i];
			
			switch(aMovie->audioFrame->ch_layout.order == AV_CHANNEL_ORDER_NATIVE ? aMovie->audioFrame->ch_layout.u.mask : 0)
			{
				case AV_CH_LAYOUT_MONO:
					aMovie->alFormat = (aMovie->audioFrame->format == AV_SAMPLE_FMT_U8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
					aMovie->alNumChannels = 1;
					break;
				case AV_CH_LAYOUT_STEREO:
					aMovie->alFormat = (aMovie->audioFrame->format == AV_SAMPLE_FMT_U8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
					aMovie->alNumChannels = 2;
					break;
			}
			
			aMovie->alSampleRate = aMovie->audioFrame->sample_rate;
			aMovie->audioTempBuffer = malloc(aMovie->alNumChannels * aMovie->audioFrame->nb_samples * 2 * 2);
			aMovie->alInited=TRUE;
		}


		memset(aMovie->audioTempBuffer, 0, aMovie->alNumChannels * aMovie->audioFrame->nb_samples * 2 * 2);

		if(aMovie->alNumChannels==0)
			return 0;
	
		// reclaim completed frames
		int processedBuffers = 0;
		alGetSourcei(aMovie->alSource, AL_BUFFERS_PROCESSED, &processedBuffers);
		if(processedBuffers>0)
		{
			alSourceStop(aMovie->alSource);
			while(processedBuffers>0)
			{
				ALuint buffer = 0;
				alSourceUnqueueBuffers(aMovie->alSource, 1, &buffer);
				AL_CHECK();
				
				if(buffer>0)
				{
					aMovie->alFreeBuffers[aMovie->alNumFreeBuffers] = buffer;
					aMovie->alNumFreeBuffers++;
				}
				processedBuffers--;
			}
			alSourcePlay(aMovie->alSource);
		}

		// queue this frame
		if(aMovie->alNumFreeBuffers>0)
		{
			ALuint alBuffer = aMovie->alFreeBuffers[aMovie->alNumFreeBuffers-1];

			int sampleCount = aMovie->audioFrame->nb_samples * aMovie->alNumChannels;


			// 16bit is deafult
			uint dataSize = sampleCount*2;
			void* data = (void*) aMovie->audioFrame->extended_data[0];

			switch(aMovie->audioFrame->format)
			{
				case AV_SAMPLE_FMT_U8:
				{
					dataSize = sampleCount;
				} break;

				default:
				case AV_SAMPLE_FMT_S16:
				{
				/*
					unsigned short* p = (unsigned short*) data;
					for(int i=0; i<sampleCount; i++)
						p[i] -= p[i]>>2;
				*/
				} break;
			
				case AV_SAMPLE_FMT_FLT:
				{
					data = (void*) aMovie->audioTempBuffer;
					short* tempBuf = (short*) aMovie->audioTempBuffer;
					float* srcBuf = (float*) aMovie->audioFrame->extended_data[0];
					for(int i=0; i<sampleCount; i++)
					{
						float val = srcBuf[i] * 32768;
						if(val >  32767) val = 32767;
						if(val < -32768) val = 32768;
						tempBuf[i] = (short) val;
						
					}
				} break;
			
				case AV_SAMPLE_FMT_S32:
				{
					data = (void*) aMovie->audioTempBuffer;
					short* tempBuf = (short*) aMovie->audioTempBuffer;
					unsigned int* srcBuf = (unsigned int*) aMovie->audioFrame->extended_data[0];
					for(int i=0; i<sampleCount; i++)
						tempBuf[i] = (short) (((*srcBuf - *srcBuf>>2) >> 16) & 0x0000FFFF);
				} break;
				
				case AV_SAMPLE_FMT_FLTP:
				{
					data = (void*) aMovie->audioTempBuffer;
					short* tempBuf = (short*) aMovie->audioTempBuffer;
						
					for(int i=0; i<aMovie->audioFrame->nb_samples; i++)
					{
						for(int j=0; j<aMovie->alNumChannels; j++)
						{
							float* srcBuf = (float*) aMovie->audioFrame->extended_data[j];
							float val = srcBuf[i] * 32768;
							if(val >  32767) val = 32767;
							if(val < -32768) val = 32768;
							tempBuf[(i*aMovie->alNumChannels)+j] = (short) val;
						}
					}
				} break;
			}
			
			alSourceStop(aMovie->alSource);

			//printf("fmt=%d, buffer size=%d, rdy=%d, len=%d, s1=%d, samples=%d\n", aMovie->audioFrame->format, dataSize, decoded_frame_ready, len, aPacket->size, sampleCount);

			alBufferData(alBuffer, aMovie->alFormat, data, dataSize-16, aMovie->alSampleRate);
			AL_CHECK();
			
			alSourceQueueBuffers(aMovie->alSource, 1, &alBuffer);
			AL_CHECK();

			float vx, vy, vz;
			alGetListener3f(AL_VELOCITY, &vx, &vy, &vz);
			alSource3f(aMovie->alSource, AL_VELOCITY, vx, vy, vz);

			alSourcePlay(aMovie->alSource);
			
			aMovie->alNumFreeBuffers--;
			aMovie->alFreeBuffers[aMovie->alNumFreeBuffers] = 0;
		}
	}
	return 1;
}



static int BinkConsumeVideo(void* opaque, AVFrame* frame)
{
	struct binkMovie* movie = opaque;
	av_frame_unref(movie->videoFrame);
	av_frame_move_ref(movie->videoFrame, frame);
	return BinkProcessFrame(movie, 1) ? 0 : AVERROR_EXIT;
}

static int BinkConsumeAudio(void* opaque, AVFrame* frame)
{
	struct binkMovie* movie = opaque;
	av_frame_unref(movie->audioFrame);
	av_frame_move_ref(movie->audioFrame, frame);
	return BinkProcessFrame(movie, 0) ? 0 : AVERROR_EXIT;
}

int BinkDecodeFrame(struct binkMovie* aMovie)
{
	AVPacket* packet = aMovie->packet;
	av_packet_unref(packet);
	if(av_read_frame(aMovie->avContext, packet) < 0)
	{
		int frames = 0;
		if(aMovie->videoCodecContext)
		{
			int count = BinkDecodePacket(aMovie->videoCodecContext, NULL, aMovie->decodeFrame, BinkConsumeVideo, aMovie);
			if(count > 0) frames += count;
		}
		if(aMovie->audioCodecContext)
		{
			int count = BinkDecodePacket(aMovie->audioCodecContext, NULL, aMovie->decodeFrame, BinkConsumeAudio, aMovie);
			if(count > 0) frames += count;
		}
		av_packet_unref(packet);
		if(frames > 0)
			return 1;
		if(!aMovie->looping || av_seek_frame(aMovie->avContext, -1, 0, 0) < 0)
			return 0;
		if(aMovie->videoCodecContext)
			avcodec_flush_buffers(aMovie->videoCodecContext);
		if(aMovie->audioCodecContext)
			avcodec_flush_buffers(aMovie->audioCodecContext);
		if(av_read_frame(aMovie->avContext, packet) < 0)
		{
			av_packet_unref(packet);
			return 0;
		}
	}

	int result = 0;
	if(packet->stream_index == aMovie->videoStreamIndex)
		result = BinkDecodePacket(aMovie->videoCodecContext, packet, aMovie->decodeFrame, BinkConsumeVideo, aMovie);
	else if(packet->stream_index == aMovie->audioStreamIndex)
		result = BinkDecodePacket(aMovie->audioCodecContext, packet, aMovie->decodeFrame, BinkConsumeAudio, aMovie);
	av_packet_unref(packet);
	if(result == AVERROR_EXIT)
		return 0;
	return result < 0 ? aMovie->looping : 1;
}


int BinkUpdateMovie(struct binkMovie* aMovie)
{
	if(!aMovie->avContext)
		return 0;

	uint timeNow = SDL_GetTicks();
	float delta = ((float)(timeNow - aMovie->timeLastUpdate)) / 1000.0f;
	
	
	if(aMovie->videoStreamIndex >= 0)
	{
		if(delta < (1.0f / aMovie->videoFrameRate))
		{
			if(aMovie->audioStreamIndex<0 || !SoundSys_IsOn() || aMovie->alNumFreeBuffers==0)
				return 1;
		}
	}
	else if(aMovie->audioStreamIndex >= 0 && aMovie->alInited)
	{
		int processedBuffers = 0;
		alGetSourcei(aMovie->alSource, AL_BUFFERS_PROCESSED, &processedBuffers);
		if(aMovie->alNumFreeBuffers<=0 && processedBuffers == 0)
			return 1;
	}
	
	int streamsPlaying = 0;
	if(aMovie->videoStreamIndex >= 0)
		streamsPlaying += BinkDecodeFrame(aMovie);
	
	if(aMovie->audioStreamIndex >= 0)	
		streamsPlaying += BinkDecodeFrame(aMovie);

	aMovie->timeLastUpdate = timeNow;
	return (streamsPlaying > 0) ? 1 : 0;
}



//-----------------------------------------------------------------------------------------------


BOOL BinkSys_Init()
{
	if(binkInitialized)
		return TRUE;

	binkInitialized = TRUE;	
	return binkInitialized;
}



void BinkSys_Release()
{
	if(!binkInitialized)
		return;
		
	binkInitialized = FALSE;
}



//-----------------------------------------------------------------------------------------------


void PlayBinkedFMV(char *filenamePtr, int volume)
{
	if(!binkInitialized)
		return;
	
	struct binkMovie movie;
	
	if(BinkStartMovie(&movie, filenamePtr, FALSE, FALSE, FALSE))
	{
		alSourcef(movie.alSource, AL_GAIN, PlatVolumeToGain(volume));
		while(BinkUpdateMovie(&movie))
		{
			BinkRenderMovie(&movie);
			FlipBuffers();
		}
		BinkReleaseMovie(&movie);
	}
}


//-----------------------------------------------------------------------------------------------

struct binkMovie menuBackgroundMovie;

void StartMenuBackgroundBink()
{
	if(!binkInitialized)
		return;

	BinkStartMovie(&menuBackgroundMovie, "FMVs/Menubackground.bik", TRUE, FALSE, FALSE);
}


int PlayMenuBackgroundBink()
{
	if(!binkInitialized)
		return 0;

	ClearScreenToBlack();
	if(BinkUpdateMovie(&menuBackgroundMovie))
	{
		BinkRenderMovie(&menuBackgroundMovie);
		return 1;
	}
	return 0;
}


void EndMenuBackgroundBink()
{
	if(!binkInitialized)
		return;

	BinkReleaseMovie(&menuBackgroundMovie);
}


//-----------------------------------------------------------------------------------------------

struct binkMovie musicMovie;

int StartMusicBink(char* filenamePtr, BOOL looping)
{
	if(!binkInitialized || !SoundSys_IsOn())
		return 0;

	int ret = BinkStartMovie(&musicMovie, filenamePtr, looping, FALSE, TRUE);
	return ret;
}

int PlayMusicBink(int volume)
{
	if(!binkInitialized || !SoundSys_IsOn())
		return 1;

	if(!musicMovie.avContext)
		return 1;

	if(!(musicMovie.audioStreamIndex>=0 && musicMovie.alInited))
		return 1;

	alSourcef(musicMovie.alSource, AL_GAIN, PlatVolumeToGain(volume));
	for(int i=0; i<musicMovie.avContext->nb_streams * FRAMEQUEUESIZE; i++)
	{
		int processedBuffers = 0;
		alGetSourcei(musicMovie.alSource, AL_BUFFERS_PROCESSED, &processedBuffers);
		if(processedBuffers + musicMovie.alNumFreeBuffers > 0)
		{
			if(!BinkDecodeFrame(&musicMovie))
			{
				return 0;
			}
		}
	}
	return 1;
}

void EndMusicBink()
{
	if(!binkInitialized || !SoundSys_IsOn())
		return;

	BinkReleaseMovie(&musicMovie);
}


//-----------------------------------------------------------------------------------------------


FMVHandle CreateBinkFMV(char* filenamePtr)
{
	if(!binkInitialized)
		return 0;

	struct binkMovie* movie = malloc(sizeof(struct binkMovie));
	BinkInitMovieStruct(movie);

	if(!BinkStartMovie(movie, filenamePtr, FALSE, TRUE, FALSE))
	{
		free(movie);
		return 0;
	}
	return (FMVHandle)movie;
}


int UpdateBinkFMV(FMVHandle aFmvHandle, int volume)
{
	if(!binkInitialized || aFmvHandle==0)
		return 0;
	
	struct binkMovie* movie = (struct binkMovie*)aFmvHandle;
	alSourcef(movie->alSource, AL_GAIN, PlatVolumeToGain(volume));
	BinkUpdateMovie(movie);
	BinkUpdateMovie(movie);
	BinkUpdateMovie(movie);
	BinkUpdateMovie(movie);
	return BinkUpdateMovie(movie);
}


void CloseBinkFMV(FMVHandle aFmvHandle)
{
	if(!binkInitialized || aFmvHandle==0)
		return;
	
	struct binkMovie* movie = (struct binkMovie*)aFmvHandle;
	BinkReleaseMovie(movie);
	free(movie);
}


char* GetBinkFMVImage(FMVHandle aFmvHandle)
{
	if(!binkInitialized || aFmvHandle==0)
		return 0;
		
	struct binkMovie* movie = (struct binkMovie*)aFmvHandle;
	
	if(!movie->videoScaleContext)
		return 0;
		
	return movie->videoScalePicture.data[0];
}


