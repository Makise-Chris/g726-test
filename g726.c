#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>
#include <ctype.h>
#include <sndfile.h>
#include <math.h>
#include <spandsp.h>

#include </usr/include/spandsp/test_utils.h>

#define BLOCK_LEN           320
#define MAX_TEST_VECTOR_LEN 40000

#define IN_FILE_NAME        "short_nb_voice.wav"
#define OUT_FILE_NAME       "post_g726.wav"

int64_t mse=0;
int64_t sumInput=0;
int sampleCnt=0;

void updateSNR(int16_t input, int16_t output){
    sumInput = sumInput + input*input;
    mse = mse + (input-output)*(input-output);
    sampleCnt++;
}

int16_t outdata[MAX_TEST_VECTOR_LEN];
uint8_t adpcmdata[MAX_TEST_VECTOR_LEN];

int16_t itudata[MAX_TEST_VECTOR_LEN];
uint8_t itu_ref[MAX_TEST_VECTOR_LEN];
uint8_t unpacked[MAX_TEST_VECTOR_LEN];
uint8_t xlaw[MAX_TEST_VECTOR_LEN];

/*
Table 4 - Reset and homing sequences for u-law
            Normal                              I-input     Overload
Algorithm   Input   Intermediate    Output      Input       Output      Input   Intermediate    Output
            (PCM)   (ADPCM)         (PCM)       (ADPCM)     (PCM)       (PCM)   (ADPCM)         (PCM)

16F         NRM.M   RN16FM.I        RN16FM.O    I16         RI16FM.O    OVR.M   RV16FM.I        RV16FM.O
                    HN16FM.I        HN16FM.O                HI16FM.O            HV16FM.I        HV16FM.O

24F         NRM.M   RN24FM.I        RN24FM.O    I24         RI24FM.O    OVR.M   RV24FM.I        RV24FM.O
                    HN24FM.I        HN24FM.O                HI24FM.O            HV24FM.I        HV24FM.O

32F         NRM.M   RN32FM.I        RN32FM.O    I32         RI32FM.O    OVR.M   RV32FM.I        RV32FM.O
                    HN32FM.I        HN32FM.O                HI32FM.O            HV32FM.I        HV32FM.O

40F         NRM.M   RN40FM.I        RN40FM.O    I40         RI40FM.O    OVR.M   RV40FM.I        RV40FM.O
                    HN40FM.I        HN40FM.O                HI40FM.O            HV40FM.I        HV40FM.O


Table 5 - Reset and homing sequences for A-law
            Normal                              I-input     Overload
Algorithm   Input   Intermediate    Output      Input       Output      Input   Intermediate    Output
            (PCM)   (ADPCM)         (PCM)       (ADPCM)     (PCM)       (PCM)   (ADPCM)         (PCM)
16F         NRM.A   RN16FA.I        RN16FA.O    I16         RI16FA.O    OVR.A   RV16FA.I        RV16FA.O
                    HN16FA.I        HN16FA.O                HI16FA.O            HV16FA.I        HV16FA.O

24F         NRM.A   RN24FA.I        RN24FA.O    I24         RI24FA.O    OVR.A   RV24FA.I        RV24FA.O
                    HN24FA.I        HN24FA.O                HI24FA.O            HV24FA.I        HV24FA.O

32F         NRM.A   RN32FA.I        RN32FA.O    I32         RI32FA.O    OVR.A   RV32FA.I        RV32FA.O
                    HN32FA.I        HN32FA.O                HI32FA.O            HV32FA.I        HV32FA.O

40F         NRM.A   RN40FA.I        RN40FA.O    I40         RI40FA.O    OVR.A   RV40FA.I        RV40FA.O
                    HN40FA.I        HN40FA.O                HI40FA.O            HV40FA.I        HV40FA.O

Table 6 - Reset and homing cross sequences for u-law -> A-law
            Normal                              Overload
Algorithm   Input   Intermediate    Output      Input   Intermediate    Output
            (PCM)   (ADPCM)         (PCM)       (PCM)   (ADPCM)         (PCM)
16F         NRM.M   RN16FM.I        RN16FC.O    OVR.M   RV16FM.I        RV16FC.O
                    HN16FM.I        HN16FC.O            HV16FM.I        HV16FC.O

24F         NRM.M   RN24FM.I        RN24FC.O    OVR.M   RV24FM.I        RV24FC.O
                    HN24FM.I        HN24FC.O            HV24FM.I        HV24FC.O

32F         NRM.M   RN32FM.I        RN32FC.O    OVR.M   RV32FM.I        RV32FC.O
                    HN32FM.I        HN32FC.O            HV32FM.I        HV32FC.O

40F         NRM.M   RN40FM.I        RN40FC.O    OVR.M   RV40FM.I        RV40FC.O
                    HN40FM.I        HN40FC.O            HV40FM.I        HV40FC.O

Table 7 - Reset and homing cross sequences for A-law -> u-law
            Normal                              Overload
Algorithm   Input   Intermediate    Output      Input   Intermediate    Output
            (PCM)   (ADPCM)         (PCM)       (PCM)   (ADPCM)         (PCM)
16F         NRM.A   RN16FA.I        RN16FX.O    OVR.A   RV16FA.I        RV16FX.O
                    HN16FA.I        HN16FX.O            HV16FA.I        HV16FX.O

24F         NRM.A   RN24FA.I        RN24FX.O    OVR.A   RV24FA.I        RV24FX.O
                    HN24FA.I        HN24FX.O            HV24FA.I        HV24FX.O

32F         NRM.A   RN32FA.I        RN32FX.O    OVR.A   RV32FA.I        RV32FX.O
                    HN32FA.I        HN32FX.O            HV32FA.I        HV32FX.O

40F         NRM.A   RN40FA.I        RN40FX.O    OVR.A   RV40FA.I        RV40FX.O
                    HN40FA.I        HN40FX.O            HV40FA.I        HV40FX.O
*/

#define G726_ENCODING_NONE          9999
/*- End of function --------------------------------------------------------*/

#define SF_MAX_HANDLE   32

static int sf_close_at_exit_registered = false;

static SNDFILE *sf_close_at_exit_list[SF_MAX_HANDLE] =
{
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static void sf_close_at_exit(void)
{
    int i;

    for (i = 0;  i < SF_MAX_HANDLE;  i++)
    {
        if (sf_close_at_exit_list[i])
        {
            sf_close(sf_close_at_exit_list[i]);
            sf_close_at_exit_list[i] = NULL;
        }
    }
}
/*- End of function --------------------------------------------------------*/

static int sf_record_handle(SNDFILE *handle)
{
    int i;

    for (i = 0;  i < SF_MAX_HANDLE;  i++)
    {
        if (sf_close_at_exit_list[i] == NULL)
            break;
    }
    if (i >= SF_MAX_HANDLE)
        return -1;
    sf_close_at_exit_list[i] = handle;
    if (!sf_close_at_exit_registered)
    {
        atexit(sf_close_at_exit);
        sf_close_at_exit_registered = true;
    }
    return 0;
}
/*- End of function --------------------------------------------------------*/

SPAN_DECLARE(SNDFILE *) sf_open_telephony_read(const char *name, int channels)
{
    SNDFILE *handle;
    SF_INFO info;

    memset(&info, 0, sizeof(info));
    if ((handle = sf_open(name, SFM_READ, &info)) == NULL)
    {
        fprintf(stderr, "    Cannot open audio file '%s' for reading\n", name);
        exit(2);
    }
    if (info.samplerate != SAMPLE_RATE)
    {
        printf("    Unexpected sample rate in audio file '%s'\n", name);
        exit(2);
    }
    if (info.channels != channels)
    {
        printf("    Unexpected number of channels in audio file '%s'\n", name);
        exit(2);
    }
    sf_record_handle(handle);
    return handle;
}
/*- End of function --------------------------------------------------------*/

SPAN_DECLARE(SNDFILE *) sf_open_telephony_write(const char *name, int channels)
{
    SNDFILE *handle;
    SF_INFO info;

    memset(&info, 0, sizeof(info));
    info.frames = 0;
    info.samplerate = SAMPLE_RATE;
    info.channels = channels;
    info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    info.sections = 1;
    info.seekable = 1;

    if ((handle = sf_open(name, SFM_WRITE, &info)) == NULL)
    {
        fprintf(stderr, "    Cannot open audio file '%s' for writing\n", name);
        exit(2);
    }
    sf_record_handle(handle);
    return handle;
}
/*- End of function --------------------------------------------------------*/

SPAN_DECLARE(int) sf_close_telephony(SNDFILE *handle)
{
    int res;
    int i;

    if ((res = sf_close(handle)) == 0)
    {
        for (i = 0;  i < SF_MAX_HANDLE;  i++)
        {
            if (sf_close_at_exit_list[i] == handle)
            {
                sf_close_at_exit_list[i] = NULL;
                break;
            }
        }
    }
    return res;
}
/*- End of function --------------------------------------------------------*/


int main(int argc, char *argv[])
{
    g726_state_t *enc_state;
    g726_state_t *dec_state;
    int opt;
    bool itutests;
    int bit_rate;
    SNDFILE *inhandle;
    SNDFILE *outhandle;
    int16_t amp[1024];
    int16_t amp_out[1024];
    int frames;
    int adpcm;
    int packing;

    bit_rate = 16000;
    packing = G726_PACKING_NONE;

    if ((inhandle = sf_open_telephony_read(IN_FILE_NAME, 1)) == NULL)
    {
        fprintf(stderr, "    Cannot open audio file '%s'\n", IN_FILE_NAME);
        exit(2);
    }
    if ((outhandle = sf_open_telephony_write(OUT_FILE_NAME, 1)) == NULL)
    {
        fprintf(stderr, "    Cannot create audio file '%s'\n", OUT_FILE_NAME);
        exit(2);
    }

    printf("ADPCM packing is %d\n", packing);
    enc_state = g726_init(NULL, bit_rate, G726_ENCODING_LINEAR, packing);
    dec_state = g726_init(NULL, bit_rate, G726_ENCODING_LINEAR, packing);

    while ((frames = sf_readf_short(inhandle, amp, 159)))
    {
        for(int i=0;i<159;i++){
            printf("%x||", amp[i]);
        }
        printf("\n===============================\n");
        adpcm = g726_encode(enc_state, adpcmdata, amp, frames);
        frames = g726_decode(dec_state, amp_out, adpcmdata, adpcm);
        for(int i=0;i<159;i++){
            printf("%x||", adpcmdata[i]);
        }
        printf("\n===============================\n");
        for(int i=0;i<159;i++){
            printf("%x||", amp_out[i]);
            updateSNR(amp[i], amp_out[i]);
        }
        printf("\n----------------------------------------------------------\n");
        sf_writef_short(outhandle, amp, frames);
    }
    if (sf_close_telephony(inhandle))
    {
        printf("    Cannot close audio file '%s'\n", IN_FILE_NAME);
        exit(2);
    }
    if (sf_close_telephony(outhandle))
    {
        printf("    Cannot close audio file '%s'\n", OUT_FILE_NAME);
        exit(2);
    }
    printf("'%s' transcoded to '%s' at %dbps.\n", IN_FILE_NAME, OUT_FILE_NAME, bit_rate);
    float snr = 10*log10f(sumInput/(mse*1.0f));
    //float snr = mse/(sampleCnt*1.0f);
    printf("Do ton hao: %f\n", snr);
    printf("So luong mau: %d\n", sampleCnt);
    g726_free(enc_state);
    g726_free(dec_state);

    return 0;
}
/*- End of function --------------------------------------------------------*/
/*- End of file ------------------------------------------------------------*/