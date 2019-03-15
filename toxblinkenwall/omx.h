/**
 *
 * ToxBlinkenwall
 * (C)Zoff <zoff@zoff.cc> in 2017 - 2019
 * (C)strfry <mail@strfry.org> in 2018 - 2019
 *
 * https://github.com/zoff99/ToxBlinkenwall
 *
 */

struct omx_display;

enum display_result {
	DISPLAY_OK = 0,
	DISPLAY_INVALID_VALUE,
	DISPLAY_INTERNAL_ERROR,
	/* ... */
};

display_result omx_display_create(struct omx_display*);
display_result omx_display_destroy(struct omx_display*);

display_result omx_display_set_input_size(struct omx_display*, int width, int height, int stride);

display_result omx_display_set_output_rect(struct omx_display*, int x_offset, int y_offset, int width, int height);
display_result omx_display_set_output_rotation(struct omx_display*, unsigned angle);

display_result omx_display_lock_buffer(struct omx_display*, void** bufp);
display_result omx_display_unlock_buffer(struct omx_display*, void* bufp);
