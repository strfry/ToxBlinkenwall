
#include  "bcm_host.h"

///
//  WinCreate() - RaspberryPi, direct surface (No X, Xlib)
//
//      This function initialized the display and window for EGL
//
EGLBoolean WinCreate(ESContext *esContext, const char *title, int display_width, int display_height, int frame_width, int frame_height)
{
   int32_t success = 0;

   static EGL_DISPMANX_WINDOW_T nativewindow;

   DISPMANX_ELEMENT_HANDLE_T dispman_element;
   DISPMANX_DISPLAY_HANDLE_T dispman_display;
   DISPMANX_UPDATE_HANDLE_T dispman_update;
   VC_RECT_T dst_rect;
   VC_RECT_T src_rect;
   
   // create an EGL window surface, passing context width/height
   success = graphics_get_display_size(0 /* LCD */, &display_width, &display_height);
   if ( success < 0 )
   {
      return EGL_FALSE;
   }

   dst_rect.x = 0;
   dst_rect.y = 0;
   dst_rect.width = display_width;
   dst_rect.height = display_height;
      
   src_rect.x = 0;
   src_rect.y = 0;
   src_rect.width = frame_width;
   src_rect.height = frame_height;   

   dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
   dispman_update = vc_dispmanx_update_start( 0 );
         
   dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
      0/*layer*/, &dst_rect, 0/*src*/,
      &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, 0/*transform*/);
      
   nativewindow.element = dispman_element;
   nativewindow.width = display_width;
   nativewindow.height = display_height;
   vc_dispmanx_update_submit_sync( dispman_update );
   
   esContext->hWnd = &nativewindow;

	return EGL_TRUE;
}
///
//  userInterrupt()
//
//      Reads from X11 event loop and interrupt program if there is a keypress, or
//      window close action.
//
GLboolean userInterrupt(ESContext *esContext)
{
	//GLboolean userinterrupt = GL_FALSE;
    //return userinterrupt;
    
    // Ctrl-C for now to stop
    
    return GL_FALSE;
}
