#ifndef AKIT_TRACK_H
#define AKIT_TRACK_H
#include <akit/sound.h>
#include <mac/buffer.h>
#include <stdbool.h>
#include <akit/plugin.h>

struct AKIT_ENGINE_STRUCT;

typedef struct {
  AkitSoundClipList clips;
  pthread_mutex_t clips_mutex;
  AkitPluginBuffer plugins;
  bool initialized;
} AkitTrack;


int akit_track_init(AkitTrack* track);

int akit_track_push(AkitTrack* track, AkitSoundClip clip);

int akit_track_push_plugin(AkitTrack* track, AkitPlugin plugin);

int akit_track_process_block(struct AKIT_ENGINE_STRUCT* engine, AkitTrack* track, float* buffer, int64_t length, int64_t frame, double time);

void akit_track_destroy(AkitTrack* track);

MAC_DEFINE_BUFFER(AkitTrack);

#endif
