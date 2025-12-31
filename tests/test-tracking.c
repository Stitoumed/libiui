/*
 * Per-Frame Field ID Tracking Tests
 *
 * Tests for stale state prevention when widgets are conditionally hidden.
 */

#include "common.h"

/* Test textfield registration */
static void test_textfield_registration(void)
{
    TEST(textfield_registration);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[32] = "Test";
    size_t cursor = 0;

    /* Render a frame with the textfield */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);

    /* Before end_frame, field should be registered */
    ASSERT_TRUE(iui_textfield_is_registered(ctx, text_buf));
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test slider registration */
static void test_slider_registration(void)
{
    TEST(slider_registration);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    float value = 0.5f;

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_slider_ex(ctx, value, 0.0f, 1.0f, 0.1f, NULL);
    iui_end_window(ctx);

    /* At least one slider should be registered */
    ASSERT_TRUE(ctx->field_tracking.slider_count > 0);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test stale textfield state cleared when not rendered */
static void test_textfield_stale_state_cleared(void)
{
    TEST(textfield_stale_state_cleared);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[32] = "Hello";
    size_t cursor = 2;

    /* Frame 1: Render textfield and focus it */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    /* Verify textfield is focused */
    ASSERT_EQ(ctx->focused_edit, text_buf);

    /* Frame 2: Do NOT render the textfield (conditionally hidden) */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    /* textfield intentionally not rendered */
    iui_button(ctx, "Other", IUI_ALIGN_CENTER);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Focused edit should be cleared since textfield wasn't rendered */
    ASSERT_NULL(ctx->focused_edit);

    free(buffer);
    PASS();
}

/* Test slider stale state cleared when not rendered */
static void test_slider_stale_state_cleared(void)
{
    TEST(slider_stale_state_cleared);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    float value = 50.0f;

    /* Frame 1: Render slider and start dragging */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    value = iui_slider_ex(ctx, value, 0.0f, 100.0f, 1.0f, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Frame 2: Keep dragging */
    iui_update_mouse_pos(ctx, 220.0f, 150.0f);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    value = iui_slider_ex(ctx, value, 0.0f, 100.0f, 1.0f, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Frame 3: Do NOT render the slider (conditionally hidden) */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    /* slider intentionally not rendered */
    iui_button(ctx, "Other", IUI_ALIGN_CENTER);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Active slider state should be cleared */
    ASSERT_EQ(ctx->slider.active_id & IUI_SLIDER_ID_MASK, 0);

    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);
    free(buffer);
    PASS();
}

/* Test multiple textfields per frame */
static void test_multiple_textfields(void)
{
    TEST(multiple_textfields);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char buf1[32] = "One";
    char buf2[32] = "Two";
    char buf3[32] = "Three";
    size_t c1 = 0, c2 = 0, c3 = 0;

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_textfield(ctx, buf1, sizeof(buf1), &c1, NULL);
    iui_textfield(ctx, buf2, sizeof(buf2), &c2, NULL);
    iui_textfield(ctx, buf3, sizeof(buf3), &c3, NULL);
    iui_end_window(ctx);

    /* All three should be registered */
    ASSERT_TRUE(iui_textfield_is_registered(ctx, buf1));
    ASSERT_TRUE(iui_textfield_is_registered(ctx, buf2));
    ASSERT_TRUE(iui_textfield_is_registered(ctx, buf3));
    ASSERT_EQ(ctx->field_tracking.textfield_count, 3);

    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test multiple sliders per frame */
static void test_multiple_sliders(void)
{
    TEST(multiple_sliders);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_slider_ex(ctx, 10.0f, 0.0f, 100.0f, 1.0f, NULL);
    iui_slider_ex(ctx, 50.0f, 0.0f, 100.0f, 1.0f, NULL);
    iui_slider_ex(ctx, 90.0f, 0.0f, 100.0f, 1.0f, NULL);
    iui_end_window(ctx);

    /* All three should be registered */
    ASSERT_EQ(ctx->field_tracking.slider_count, 3);

    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test frame counter increments */
static void test_frame_counter(void)
{
    TEST(frame_counter);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    uint32_t initial_frame = ctx->field_tracking.frame_number;

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_EQ(ctx->field_tracking.frame_number, initial_frame + 1);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_EQ(ctx->field_tracking.frame_number, initial_frame + 2);

    free(buffer);
    PASS();
}

/* Test tracking reset between frames */
static void test_tracking_reset_between_frames(void)
{
    TEST(tracking_reset_between_frames);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[32] = "Test";
    size_t cursor = 0;

    /* Frame 1: Register several fields */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_slider_ex(ctx, 50.0f, 0.0f, 100.0f, 1.0f, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Frame 2: Empty frame - counts should reset */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    /* After begin_frame, counts should be reset to 0 */
    ASSERT_EQ(ctx->field_tracking.textfield_count, 0);
    ASSERT_EQ(ctx->field_tracking.slider_count, 0);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test duplicate registration (same field rendered twice) */
static void test_duplicate_registration(void)
{
    TEST(duplicate_registration);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[32] = "Test";
    size_t cursor = 0;

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    /* Register same buffer twice */
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);

    /* Should only count once (deduplication) */
    ASSERT_EQ(ctx->field_tracking.textfield_count, 1);

    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test edit_with_selection also registers */
static void test_edit_with_selection_registers(void)
{
    TEST(edit_with_selection_registers);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[64] = "Hello World";
    iui_edit_state state = {0};
    state.cursor = 5;

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);

    /* edit_with_selection should register the field */
    ASSERT_TRUE(iui_textfield_is_registered(ctx, text_buf));

    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test reset_field_ids public API */
static void test_reset_field_ids_api(void)
{
    TEST(reset_field_ids_api);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[32] = "Test";

    /* Manually register a field */
    iui_register_textfield(ctx, text_buf);
    ASSERT_EQ(ctx->field_tracking.textfield_count, 1);

    /* Reset via public API */
    iui_reset_field_ids(ctx);

    /* Should be reset */
    ASSERT_EQ(ctx->field_tracking.textfield_count, 0);

    free(buffer);
    PASS();
}

/* Test Suite Runner */

void run_field_tracking_tests(void)
{
    SECTION_BEGIN("Field Tracking");
    test_textfield_registration();
    test_slider_registration();
    test_textfield_stale_state_cleared();
    test_slider_stale_state_cleared();
    test_multiple_textfields();
    test_multiple_sliders();
    test_frame_counter();
    test_tracking_reset_between_frames();
    test_duplicate_registration();
    test_edit_with_selection_registers();
    test_reset_field_ids_api();
    SECTION_END();
}
