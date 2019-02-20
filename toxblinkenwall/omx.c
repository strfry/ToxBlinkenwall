/**
 *
 * ToxBlinkenwall
 * (C)Zoff <zoff@zoff.cc> in 2017 - 2019
 *
 * https://github.com/zoff99/ToxBlinkenwall
 *
 */
/**
 * @file omx.c     Raspberry Pi VideoCoreIV OpenMAX interface
 *
 * Copyright (C) 2016 - 2017 Creytiv.com
 * Copyright (C) 2016 - 2017 Jonathan Sieber
 * Copyright (C) 2018 - 2019 Zoff <zoff@zoff.cc>
 */


#include "omx.h"

/*
 * forward func declarations (acutal functions in toxblinkenwall.c)
 */
void dbg(int level, const char *fmt, ...);
void usleep_usec(uint64_t msec);


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Avoids a VideoCore header warning about clock_gettime() */
#include <time.h>
#include <sys/time.h>
#include <semaphore.h>

#ifdef RASPBERRY_PI
    #include <bcm_host.h>
#endif

#ifdef RASPBERRY_PI
    static const int VIDEO_RENDER_PORT = 90;
#else
    static const int VIDEO_RENDER_PORT = 0;
#endif


struct omx_buffer {
    OMX_BUFFERHEADERTYPE* header;
    sem_t semaphore;
};

static OMX_ERRORTYPE EventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
                                  OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2,
                                  OMX_PTR pEventData)
{
    (void) hComponent;

    switch (eEvent)
    {
        case OMX_EventCmdComplete:
            dbg(9, "omx.EventHandler: Previous command completed\n"
                "d1=%x\td2=%x\teventData=%p\tappdata=%p\n",
                nData1, nData2, pEventData, pAppData);
            /* TODO: Put these event into a multithreaded queue,
             * properly wait for them in the issuing code */
            break;

        case OMX_EventError:
            dbg(9, "omx.EventHandler: Error event type "
                "data1=%x\tdata2=%x\n", nData1, nData2);
            break;

        default:
            dbg(9, "omx.EventHandler: Unknown event type %d\t"
                "data1=%x data2=%x\n", eEvent, nData1, nData2);
            return -1;
            break;
    }

    return 0;
}


static OMX_ERRORTYPE EmptyBufferDone(OMX_HANDLETYPE hComponent,
                                     OMX_PTR pAppData,
                                     OMX_BUFFERHEADERTYPE *pBuffer)
{
    (void) hComponent;
    (void) pAppData;
    (void) pBuffer;

    struct omx_buffer* buffer = pBuffer->pAppPrivate;
    sem_post(&buffer->semaphore);

    return 0;
}

static OMX_ERRORTYPE FillBufferDone(OMX_HANDLETYPE hComponent,
                                    OMX_PTR pAppData, OMX_BUFFERHEADERTYPE *pBuffer)
{
    (void) hComponent;
    (void) pAppData;
    (void) pBuffer;
    dbg(9, "FillBufferDone\n");
    return 0;
}


static struct OMX_CALLBACKTYPE callbacks =
{
    EventHandler,
    EmptyBufferDone,
    &FillBufferDone
};


int omx_init(struct omx_state *st)
{
    OMX_ERRORTYPE err;
    bcm_host_init();
    st->buffers = NULL;
    err = OMX_Init();
    err |= OMX_GetHandle(&st->video_render,
                         "OMX.broadcom.video_render", 0, &callbacks);

    if (!st->video_render || err != 0)
    {
        dbg(9, "omx: Failed to create OMX video_render component\n");
        return ENOENT;
    }
    else
    {
        dbg(9, "omx: created video_render component\n");
        return 0;
    }
}


/* Some busy loops to verify we're running in order */
static void block_until_state_changed(OMX_HANDLETYPE hComponent,
                                      OMX_STATETYPE wanted_eState)
{
    OMX_STATETYPE eState;
    unsigned int i = 0;
    uint32_t loop_counter = 0;

    while (i++ == 0 || eState != wanted_eState)
    {
        OMX_GetState(hComponent, &eState);

        if (eState != wanted_eState)
        {
            usleep_usec(10000);
        }

        loop_counter++;

        if (loop_counter > 50)
        {
            // we don't want to get stuck here
            break;
        }
    }
}


void omx_deinit(struct omx_state *st)
{
    if (!st)
    {
        return;
    }

    OMX_SendCommand(st->video_render,
                    OMX_CommandStateSet, OMX_StateIdle, NULL);
    block_until_state_changed(st->video_render, OMX_StateIdle);
    dbg(9, "omx_deinit:003\n");
    OMX_SendCommand(st->video_render,
                    OMX_CommandStateSet, OMX_StateLoaded, NULL);
    dbg(9, "omx_deinit:004\n");
    usleep_usec(150000);
    dbg(9, "omx_deinit:005\n");
    OMX_FreeHandle(st->video_render);
    dbg(9, "omx_deinit:006\n");
    OMX_Deinit();
    dbg(9, "omx_deinit:099\n");
}


void omx_display_disable(struct omx_state *st)
{
    dbg(9, "omx_display_disable\n");
    OMX_ERRORTYPE err;
    OMX_CONFIG_DISPLAYREGIONTYPE config;

    if (!st)
    {
        return;
    }

    memset(&config, 0, sizeof(OMX_CONFIG_DISPLAYREGIONTYPE));
    config.nSize = sizeof(OMX_CONFIG_DISPLAYREGIONTYPE);
    config.nVersion.nVersion = OMX_VERSION;
    config.nPortIndex = VIDEO_RENDER_PORT;
    config.fullscreen = OMX_FALSE;
    config.set = OMX_DISPLAY_SET_FULLSCREEN;
    err = OMX_SetParameter(st->video_render,
                           OMX_IndexConfigDisplayRegion, &config);

    if (err != 0)
    {
        dbg(9, "omx_display_disable command failed\n");
    }
}


static void block_until_port_changed(OMX_HANDLETYPE hComponent,
                                     OMX_U32 nPortIndex, OMX_BOOL bEnabled)
{
    OMX_ERRORTYPE r;
    OMX_PARAM_PORTDEFINITIONTYPE portdef;
    OMX_U32 i = 0;
    memset(&portdef, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    portdef.nVersion.nVersion = OMX_VERSION;
    portdef.nPortIndex = nPortIndex;
    uint32_t    loop_counter = 0;

    while (i++ == 0 || portdef.bEnabled != bEnabled)
    {
        r = OMX_GetParameter(hComponent,
                             OMX_IndexParamPortDefinition, &portdef);

        if (r != OMX_ErrorNone)
        {
            dbg(9, "block_until_port_changed: OMX_GetParameter "
                " failed with Result=%d\n", r);
        }

        if (portdef.bEnabled != bEnabled)
        {
            usleep_usec(10000);
        }

        loop_counter++;

        if (loop_counter > 50)
        {
            // we don't want to get stuck here
            break;
        }
    }
}

int omx_change_video_out_rotation(struct omx_state *st, int angle)
{
    unsigned int i;
    OMX_PARAM_PORTDEFINITIONTYPE portdef;
    OMX_CONFIG_DISPLAYREGIONTYPE config;
    OMX_ERRORTYPE err = 0;
    memset(&config, 0, sizeof(OMX_CONFIG_DISPLAYREGIONTYPE));
    config.nSize = sizeof(OMX_CONFIG_DISPLAYREGIONTYPE);
    config.nVersion.nVersion = OMX_VERSION;
    config.nPortIndex = VIDEO_RENDER_PORT;

    if (angle == 90)
    {
        config.transform = OMX_DISPLAY_ROT90;
    }
    else if (angle == 180)
    {
        config.transform = OMX_DISPLAY_ROT180;
    }
    else if (angle == 270)
    {
        config.transform = OMX_DISPLAY_ROT270;
    }
    else
    {
        // for any other angle value just assume "0" degrees
        config.transform = OMX_DISPLAY_ROT0;
    }

    //
    config.set = (OMX_DISPLAYSETTYPE)(OMX_DISPLAY_SET_TRANSFORM);
    //
    err |= OMX_SetParameter(st->video_render,
                            OMX_IndexConfigDisplayRegion, &config);
    return err;
}

int omx_display_enable(struct omx_state *st,
                       int width, int height, int stride)
{
    unsigned int i;
    OMX_PARAM_PORTDEFINITIONTYPE portdef;
    OMX_CONFIG_DISPLAYREGIONTYPE config;
    OMX_ERRORTYPE err = 0;
    dbg(9, "omx_update_size %d %d %d\n", width, height, stride);
    memset(&config, 0, sizeof(OMX_CONFIG_DISPLAYREGIONTYPE));
    config.nSize = sizeof(OMX_CONFIG_DISPLAYREGIONTYPE);
    config.nVersion.nVersion = OMX_VERSION;
    config.nPortIndex = VIDEO_RENDER_PORT;
    //
    //
    // #define DEBUG_VIDEO_IN_FRAME 1
    //
    //
#ifdef DEBUG_VIDEO_IN_FRAME
    config.dest_rect.x_offset  = 0;
    config.dest_rect.y_offset  = 0;
    config.dest_rect.width     = (int)(1920 / 2);
    config.dest_rect.height    = (int)(1080 / 2);
    config.mode = OMX_DISPLAY_MODE_LETTERBOX;
    config.transform = OMX_DISPLAY_ROT0;
    config.fullscreen = OMX_FALSE;
    config.set = (OMX_DISPLAYSETTYPE)(OMX_DISPLAY_SET_TRANSFORM | OMX_DISPLAY_SET_DEST_RECT |
                                      OMX_DISPLAY_SET_FULLSCREEN | OMX_DISPLAY_SET_MODE);
#else
    config.fullscreen = OMX_TRUE;
    config.mode = OMX_DISPLAY_MODE_LETTERBOX;
    config.set = (OMX_DISPLAYSETTYPE)(OMX_DISPLAY_SET_FULLSCREEN | OMX_DISPLAY_SET_MODE);
#endif
    //
    err |= OMX_SetParameter(st->video_render,
                            OMX_IndexConfigDisplayRegion, &config);

    if (err != 0)
    {
        dbg(9, "omx_display_enable: couldn't configure display region\n");
    }

    memset(&portdef, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    portdef.nVersion.nVersion = OMX_VERSION;
    portdef.nPortIndex = VIDEO_RENDER_PORT;
    /* specify buffer requirements */
    err |= OMX_GetParameter(st->video_render,
                            OMX_IndexParamPortDefinition, &portdef);

    if (err != 0)
    {
        dbg(9, "omx_display_enable: couldn't retrieve port def\n");
        err = ENOMEM;
        goto exit;
    }

    //
    dbg(9, "omx port definition(before): h=%d w=%d s=%d sh=%d\n",
        portdef.format.video.nFrameWidth,
        portdef.format.video.nFrameHeight,
        portdef.format.video.nStride,
        portdef.format.video.nSliceHeight);
    //
    portdef.format.video.nFrameWidth = width;
    portdef.format.video.nFrameHeight = height;
    portdef.format.video.nStride = stride;
    portdef.format.video.nSliceHeight = height;
    //
    portdef.bEnabled = 1;
    //
    dbg(9, "omx port definition(after): h=%d w=%d s=%d sh=%d\n",
        portdef.format.video.nFrameWidth,
        portdef.format.video.nFrameHeight,
        portdef.format.video.nStride,
        portdef.format.video.nSliceHeight);
    //
    err |= OMX_SetParameter(st->video_render,
                            OMX_IndexParamPortDefinition, &portdef);

    if (err)
    {
        dbg(9, "omx_display_enable: could not set port definition\n");
        goto exit;
    }

    block_until_port_changed(st->video_render, VIDEO_RENDER_PORT, 1);
    err |= OMX_GetParameter(st->video_render,
                            OMX_IndexParamPortDefinition, &portdef);

    if (portdef.format.video.nFrameWidth != width ||
            portdef.format.video.nFrameHeight != height ||
            portdef.format.video.nStride != stride)
    {
        dbg(9, "could not set requested resolution\n");
        err = EINVAL;
        goto exit;
    }

    if (err != 0 || !portdef.bEnabled)
    {
        dbg(9, "omx_display_enable: failed to set up video port\n");
        err = ENOMEM;
        goto exit;
    }

    /* HACK: This state-change sometimes hangs for unknown reasons,
     *       so we just send the state command and wait 50 ms */
    /* block_until_state_changed(st->video_render, OMX_StateIdle); */
    OMX_SendCommand(st->video_render, OMX_CommandStateSet,
                    OMX_StateIdle, NULL);
    usleep_usec(150000);

    if (!st->buffers)
    {
        st->buffers =
            malloc(portdef.nBufferCountActual * sizeof(struct omx_buffer));
        st->num_buffers = portdef.nBufferCountActual;
        st->current_buffer = 0;

        for (i = 0; i < portdef.nBufferCountActual; i++)
        {
            err = OMX_AllocateBuffer(st->video_render,
                                     &st->buffers[i].header, VIDEO_RENDER_PORT,
                                     st, portdef.nBufferSize);

            if (err)
            {
                dbg(9, "OMX_AllocateBuffer failed: %d\n",
                    err);
                err = ENOMEM;
                goto exit;
            }

            st->buffers[i].header->pAppPrivate = &st->buffers[i];
            sem_init(&st->buffers[i].semaphore, 0, 1);
        }
    }

    dbg(9, "omx_update_size: send to execute state\n");
    OMX_SendCommand(st->video_render, OMX_CommandStateSet,
                    OMX_StateExecuting, NULL);
    block_until_state_changed(st->video_render, OMX_StateExecuting);
exit:
    return err;
}

int omx_display_input_buffer(struct omx_state *st,
                             void **pbuf, uint32_t *plen)
{
    struct omx_buffer* cur_buf = &st->buffers[st->current_buffer];

    if (!st->buffers)
    {
        return EINVAL;
    }

    *pbuf = cur_buf->header->pBuffer;
    *plen = cur_buf->header->nAllocLen;
    cur_buf->header->nFilledLen = *plen;
    cur_buf->header->nOffset = 0;

    return 0;
}

int omx_display_flush_buffer(struct omx_state *st, uint32_t data_len)
{
    struct timespec wait_time;
    struct omx_buffer* cur_buf = &st->buffers[st->current_buffer];

    clock_gettime(CLOCK_REALTIME, &wait_time);
    wait_time.tv_sec += 1;

    if (sem_timedwait(&cur_buf->semaphore, &wait_time)) {
        dbg(9, "omx_display_flush_buffer: buffer not empty after 1000 ms\n");
        return -ETIMEDOUT;
    }

    if (OMX_EmptyThisBuffer(st->video_render, cur_buf->header)
            != OMX_ErrorNone)
    {
        dbg(9, "OMX_EmptyThisBuffer error\n");
        return -EIO;
    }

    st->current_buffer = (st->current_buffer + 1) % st->num_buffers;

    return 0;
}
