// gcc -o drm-gbm drm-gbm.c -ldrm -lgbm -lEGL -lGL -I/usr/include/libdrm

// general documentation: man drm

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>
#include <EGL/egl.h>
#include <GL/gl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define EXIT(msg) { fputs (msg, stderr); exit (EXIT_FAILURE); }

static int drm_device;

static drmModeConnector *find_connector (drmModeRes *resources) {
	// iterate the connectors
	int i;
	for (i=0; i<resources->count_connectors; i++) {
		drmModeConnector *connector = drmModeGetConnector (drm_device, resources->connectors[i]);
		// pick the first connected connector
		if (connector->connection == DRM_MODE_CONNECTED) {
			return connector;
		}
		drmModeFreeConnector (connector);
	}
	// no connector found
	return NULL;
}

static drmModeEncoder *find_encoder (drmModeRes *resources, drmModeConnector *connector) {
	if (connector->encoder_id) {
		return drmModeGetEncoder (drm_device, connector->encoder_id);
	}
	// no encoder found
	return NULL;
}

static uint32_t connector_id;
static drmModeModeInfo mode_info;
static drmModeCrtc *crtc;

static void find_display_configuration () {
	drmModeRes *resources = drmModeGetResources (drm_device);
	// find a connector
	drmModeConnector *connector = find_connector (resources);
	if (!connector) EXIT ("no connector found\n");
	// save the connector_id
	connector_id = connector->connector_id;
	// save the first mode
	mode_info = connector->modes[0];
	printf ("resolution: %ix%i\n", mode_info.hdisplay, mode_info.vdisplay);
	// find an encoder
	drmModeEncoder *encoder = find_encoder (resources, connector);
	if (!encoder) EXIT ("no encoder found\n");
	// find a CRTC
	if (encoder->crtc_id) {
		crtc = drmModeGetCrtc (drm_device, encoder->crtc_id);
	}
	drmModeFreeEncoder (encoder);
	drmModeFreeConnector (connector);
	drmModeFreeResources (resources);
}

static struct gbm_device *gbm_device;
static EGLDisplay display;
static EGLContext context;
static struct gbm_surface *gbm_surface;
static EGLSurface egl_surface;

static void setup_opengl () {
	gbm_device = gbm_create_device (drm_device);
	display = eglGetDisplay (gbm_device);
	eglInitialize (display, NULL, NULL);
	
	// create an OpenGL ES context
	eglBindAPI (EGL_OPENGL_ES_API);
	EGLint attributes[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
	EGL_NONE};
	EGLConfig config;
	EGLint num_config;
	eglChooseConfig (display, attributes, &config, 1, &num_config);
	context = eglCreateContext (display, config, EGL_NO_CONTEXT, NULL);
	
	// create the GBM and EGL surface
	gbm_surface = gbm_surface_create (gbm_device, mode_info.hdisplay, mode_info.vdisplay, GBM_BO_FORMAT_XRGB8888, GBM_BO_USE_SCANOUT|GBM_BO_USE_RENDERING);
	egl_surface = eglCreateWindowSurface (display, config, gbm_surface, NULL);
	eglMakeCurrent (display, egl_surface, egl_surface, context);
}

static struct gbm_bo *previous_bo = NULL;
static uint32_t previous_fb;

//static void swap_buffers () {
void swap_buffers () {
	eglSwapBuffers (display, egl_surface);
	struct gbm_bo *bo = gbm_surface_lock_front_buffer (gbm_surface);
	uint32_t handle = gbm_bo_get_handle (bo).u32;
	uint32_t pitch = gbm_bo_get_stride (bo);
	uint32_t fb;
	drmModeAddFB (drm_device, mode_info.hdisplay, mode_info.vdisplay, 24, 32, pitch, handle, &fb);
	drmModeSetCrtc (drm_device, crtc->crtc_id, fb, 0, 0, &connector_id, 1, &mode_info);
	
	if (previous_bo) {
		drmModeRmFB (drm_device, previous_fb);
		gbm_surface_release_buffer (gbm_surface, previous_bo);
	}
	previous_bo = bo;
	previous_fb = fb;

	puts("swapped buffers");
}


static void clean_up () {
	// set the previous crtc
	drmModeSetCrtc (drm_device, crtc->crtc_id, crtc->buffer_id, crtc->x, crtc->y, &connector_id, 1, &crtc->mode);
	drmModeFreeCrtc (crtc);
	
	if (previous_bo) {
		drmModeRmFB (drm_device, previous_fb);
		gbm_surface_release_buffer (gbm_surface, previous_bo);
	}
	
	eglDestroySurface (display, egl_surface);
	gbm_surface_destroy (gbm_surface);
	eglDestroyContext (display, context);
	eglTerminate (display);
	gbm_device_destroy (gbm_device);

	close(drm_device);
}

EGLBoolean WinCreate(ESContext *esContext, const char *title, int display_width, int display_height, int frame_width, int frame_height) {
	// ignore all the size parameters
	
	drm_device = open ("/dev/dri/card0", O_RDWR|O_CLOEXEC);
	if (!drm_device) return EGL_FALSE;
	
	find_display_configuration ();
	setup_opengl ();

	//atexit(clean_up);
	
	return EGL_TRUE;
}

GLboolean userInterrupt(ESContext *esContext)
{    
    return GL_FALSE;
}
