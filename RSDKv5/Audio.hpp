#ifndef AUDIO_H
#define AUDIO_H

#define TRACK_COUNT (0x10)
#define SFX_COUNT   (0x100)
#if !RETRO_USE_ORIGINAL_CODE
#define CHANNEL_COUNT (0x10) // 4 in the original, 16 for convenience
#else
#define CHANNEL_COUNT (0x4)
#endif

#define MAX_VOLUME (100)

struct SFXInfo {
    uint hash[4];
    Sint16 *buffer;
    size_t length;
    int unknown;
    byte maxConcurrentPlays;
    byte scope;
};

struct ChannelInfo {
    Sint16 *samplePtr;
    int pan;
    float volume;
    int speed;
    size_t sampleLength;
    int bufferPos; 
    int unknown2; 
    uint loop;
    short soundID;
    byte unknown3;
    byte state;
};

enum ChannelStates { CHANNEL_NONE, CHANNEL_SFX, CHANNEL_STREAMING, CHANNEL_STREAM_LOAD, CHANNEL_PAUSED = 0x40 };

extern SFXInfo sfxList[SFX_COUNT];
extern ChannelInfo channels[CHANNEL_COUNT];

extern bool32 audioEnabled;

extern char streamFilename[0x100];
extern byte *streamFileBuffer;
extern int streamFileSize;
extern void *streamBuffer;
extern int streamBufferSize;
extern int streamStartPos;
extern int streamLoopPoint;

bool32 InitAudioDevice();
void ReleaseAudioDevice();

#if RETRO_USING_SDL1 || RETRO_USING_SDL2
#if !RETRO_USE_ORIGINAL_CODE
// These functions did exist, but with different signatures
void ProcessMusicStream(Sint32 *stream, size_t bytes_wanted);
void ProcessAudioPlayback(void *data, Uint8 *stream, int len);
void ProcessAudioMixing(Sint32 *dst, const Sint16 *src, int len, int volume, sbyte pan);
#endif

#endif

void LoadStream(ChannelInfo *channel);
int PlayStream(const char *filename, unsigned int slot, int a3, unsigned int loopPoint, bool32 loadASync);

void LoadSfx(char *filePath, byte plays, byte scope);
inline ushort GetSFX(const char* sfxName) {
    uint hash[4];
    GEN_HASH(sfxName, hash);

    for (int s = 0; s < SFX_COUNT; ++s) {
        if (HASH_MATCH(sfxList[s].hash, hash))
            return s;
    }

    return -1; 
}
int PlaySfx(ushort sfx, uint loopPoint, int unknown);
inline void StopSfx(int sfx)
{
    for (int i = 0; i < CHANNEL_COUNT; ++i) {
        if (channels[i].soundID == sfx) {
            MEM_ZERO(channels[i]);
            channels[i].soundID = -1;
            channels[i].state = CHANNEL_NONE;
        }
    }
}
void SetChannelAttributes(byte slot, float volume, float panning, float speed);

inline void StopChannel(byte slot)
{
    if (slot < CHANNEL_COUNT) {
        if (channels[slot].state != CHANNEL_STREAM_LOAD)
            channels[slot].state = CHANNEL_NONE;
    }
}

inline void PauseChannel(byte slot)
{
    if (slot < CHANNEL_COUNT) {
        if (channels[slot].state != CHANNEL_STREAM_LOAD) {
            channels[slot].state |= CHANNEL_PAUSED;
        }
    }
}

inline void ResumeChannel(byte slot)
{
    if (slot < CHANNEL_COUNT) {
        if (channels[slot].state != CHANNEL_STREAM_LOAD) {
            channels[slot].state &= ~CHANNEL_PAUSED;
        }
    }
}

inline void PauseSound()
{
    for (int c = 0; c < CHANNEL_COUNT; ++c) {
        PauseChannel(c);
    }
}

inline void ResumeSound()
{
    for (int c = 0; c < CHANNEL_COUNT; ++c) {
        ResumeChannel(c);
    }
}

inline bool32 SfxPlaying(byte slot)
{
    for (int c = 0; c < CHANNEL_COUNT; ++c) {
        if (channels[c].state == CHANNEL_SFX && channels[c].soundID == slot)
            return true;
    }
    return false;
}

inline bool32 ChannelActive(byte slot)
{
    if (slot >= CHANNEL_COUNT)
        return false;
    else
        return channels[slot].state == CHANNEL_NONE;
}

inline bool32 GetChannelPos(byte slot)
{
    if (slot >= CHANNEL_COUNT)
        return 0;
    if (channels[slot].state == CHANNEL_SFX)
        return channels[slot].bufferPos;
    if (channels[slot].state == CHANNEL_STREAMING) {
        // if (!musicFileInfo->field_41C)
        //    return 0;
        // if (musicFileInfo->dword418 >= 0)
        //    return musicFileInfo->dword418;
    }
    return 0;
}

#endif
