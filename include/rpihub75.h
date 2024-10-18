#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>


#ifndef _GPIO_H
#define _GPIO_H 1

//////////////////////////////////////////////////////////
/*
#define RED_SCALE 1.0f
#define GREEN_SCALE 0.48f
#define BLUE_SCALE 0.61f

#define RED_GAMMA_SCALE 1.2f
#define GREEN_GAMMA_SCALE 0.92f
#define BLUE_GAMMA_SCALE 0.87f


r.99, g.99, b1.0
rg, 1.0, gg .95, bg .92

*/


#define RED_SCALE 0.99f
#define GREEN_SCALE 1.04f
#define BLUE_SCALE 1.0f

#define RED_GAMMA_SCALE 1.0f
#define GREEN_GAMMA_SCALE 1.0f
#define BLUE_GAMMA_SCALE 0.92f

#define GAMMA 1.99f


#define PANEL_WIDTH 64
#define PANEL_HEIGHT 64
#define IMG_WIDTH 64
#define IMG_HEIGHT 64

#define SERVER_PORT 22222

// global OE jitter mask, should be a prime >1031 and <=4093
// we don't want to make this too large, as it will consume memory
// and decrease L1-L3 cache locality of other data
#define JITTER_SIZE 9093 

//////////////////////////////////////////////////////////

#define CONSOLE_DEBUG 1
#define ENABLE_ASSERTS 1
#define PACKET_SIZE 1450
#define PREAMBLE 0xdeadcafe

//////////////////////////////////////////////////////////

#if ENABLE_ASSERTS
    #define ASSERT(x) assert(x)
#else
    #define ASSERT(ignore) assert(ignore)
#endif

#define MAX_BITS 64


#if PI3
    #define PERI_BASE   0x3F000000
    #define GPIO_OFFSET 0x200000
    #define RIO_OFFSET  0x2E0000 / 4
    #define PAD_OFFSET  0x2F0000 / 4
#elif PI4
    #define PERI_BASE   0xFE000000
    #define GPIO_OFFSET 0x200000
    #define RIO_OFFSET  0x2E0000 / 4
    #define PAD_OFFSET  0x2F0000 / 4
#else
    #define PERI_BASE 0x1f000D0000
    //#define PERI_BASE 0x1f00000000
    #define GPIO_OFFSET 0x00000 / 4
    #define RIO_OFFSET  0x10000 / 4
    #define PAD_OFFSET  0x20000 / 4
#endif



/** @brief  set all GPIO pins to absolute bit mask */
#define rio ((rioregs *)RIOBase)
/** @brief  XOR GPIO pins with bit mask */
#define rioXOR ((rioregs *)(RIOBase + 0x1000 / 4))
/** @brief  SET GPIO pins in bit mask (dont touch pins not in mask) */
#define rioSET ((rioregs *)(RIOBase + 0x2000 / 4))
/** @brief  CLEAR GPIO pins in bit mask (dont touch pins not in mask) */
#define rioCLR ((rioregs *)(RIOBase + 0x3000 / 4))

#define SLOW for (volatile int s=0;s<10;s++) { asm(""); }
#define SLOW2 for (volatile int s=0;s<2000;s++) { asm(""); }

// helpers for timing things...
#define PRE_TIME struct timeval start, end; gettimeofday(&start, NULL);
#define POST_TIME gettimeofday(&end, NULL); long elapsed_time = (end.tv_sec - start.tv_sec) * 1000000L + (end.tv_usec - start.tv_usec); printf("microseconds (1/1000 ms): %ld\n", elapsed_time);

/**
 * pin assignments
 */
#define ADDRESS_P0_G1 27
#define ADDRESS_P0_G2 9
#define ADDRESS_P0_B1 7
#define ADDRESS_P0_B2 10
#define ADDRESS_P0_R1 11
#define ADDRESS_P0_R2 8

#define ADDRESS_P1_G1 5
#define ADDRESS_P1_G2 13
#define ADDRESS_P1_B1 6
#define ADDRESS_P1_B2 20
#define ADDRESS_P1_R1 12
#define ADDRESS_P1_R2 19

#define ADDRESS_P2_R1 14
#define ADDRESS_P2_R2 26
#define ADDRESS_P2_G1 2
#define ADDRESS_P2_G2 16
#define ADDRESS_P2_B1 3
#define ADDRESS_P2_B2 21

#define ADDRESS_A 22
#define ADDRESS_B 23
#define ADDRESS_C 24
#define ADDRESS_D 25
#define ADDRESS_E 15
#define ADDRESS_STROBE 4
#define ADDRESS_CLK 17
#define ADDRESS_OE 18

// control pins bit masks
#define PIN_OE (1 << ADDRESS_OE)
#define PIN_LATCH (1 << ADDRESS_STROBE)
#define PIN_CLK (1 << ADDRESS_CLK)


// helpers for "boolean"
#define TRUE 1
#define FALSE 0

// add address lines as a mask useful for clearing the address lines
#define ADDRESS_LINES_MASK (0 | 1 << ADDRESS_A | 1 << ADDRESS_B | 1 << ADDRESS_C | 1 << ADDRESS_D | 1 << ADDRESS_E)
#define ADDRESS_COLOR_MASK (0 | 1 << ADDRESS_P0_B1 | 1 << ADDRESS_P0_B2 | 1 << ADDRESS_P0_G1 | 1 << ADDRESS_P0_G2 | 1 << ADDRESS_P0_R1 | 1 << ADDRESS_P0_R2)


/**
 * @brief just a float, should be normalized to 0-1
 */
typedef float Normal;

/**
 * @brief pointer to a single 24bpp RGB pixel (3 bytes)
 * RGB *pixel = (RGB *)(image + offset)
 */
typedef struct {
    uint8_t r;
    uint8_t g; 
    uint8_t b; 
} RGB;

/**
 * @brief pointer to a single 24bpp RGBA pixel (4 bytes)
 * RGBA *pixel = (RGBA *)(image + offset)
 */
typedef struct {
    uint8_t r;
    uint8_t g; 
    uint8_t b; 
    uint8_t a; 
} RGBA;


/**
 * @brief pointer to a single 24bpp RGB pixel normalized as floats (0-1)
 * RGBF *pixel_norm = normalize_rgb((RGB *)(image + offset))
 */
typedef struct {
    Normal r;
    Normal g; 
    Normal b; 
} RGBF;

/**
 * @brief network packet structure
 * 
 */
struct udp_packet {
    uint32_t preamble;
    uint16_t packet_id;
    uint16_t total_packets;
    uint16_t frame_num;
    uint8_t data[PACKET_SIZE - 10];
};


// self referencing function pointers need this defined first
struct scene_info;

// void map_byte_image_to_pwm(uint8_t *image, const scene_info *scene, uint8_t fps_sync) {
typedef void (*func_pwm_mapper_t)(uint8_t *image, const struct scene_info *scene, const uint8_t fps_sync);
typedef void (*func_tone_mapper_t)(const RGBF *in, RGBF *out);
typedef uint8_t *(*func_image_mapper_t)(const uint8_t *image_in, uint8_t *image_out, const struct scene_info *scene);
typedef uint8_t *(image_mapper_t)(const uint8_t *image_in, uint8_t *image_out, const struct scene_info *scene);


/**
 * @brief everything to define the scene and panel configuration 
 * This is a kind of global configuration for the entire system 
 */
typedef struct scene_info {
    /** @brief the total width of the image in pixels */
    uint16_t width;
    /** @brief the total height of the image in pixels */
    uint16_t height;
    /** @brief the number of bytes per pixel in the drawing buffers (3 for RGB, 4 for RGBA) */
    uint8_t  stride;

    /** @brief single panel width in pixels */
    uint16_t panel_width;
    /** @brief single panel height in pixels */
    uint16_t panel_height;

    /** @brief number of ports connected to the PI (1-3) */
    uint8_t num_ports;

    /** @brief number of bits per color channel (8-32) */
    uint8_t bit_depth;

    /** @brief brightness level (0-255) */
    uint8_t brightness;

    /** 
     * @brief number of panels connected to each chain on the port (1-8)
     * 4 64 pixel panels at 32bit pwm yields 75 FPS. 8 panels at 32bit pwm yields 37 FPS.
     */
    uint8_t num_chains;

    /**
     * @brief points to active buffer
     * in update code use this to select the pwm buffer to render to: 
     * (scene->buffer_ptr == 1)? scene->pwm_signalA : scene->pwm_signalB; 
     */
    uint8_t buffer_ptr;

    /** * @brief see buffer_ptr for usage */
    uint32_t *restrict pwm_signalA __attribute__((aligned(16)));

    /** * @brief see buffer_ptr for usage */
    uint32_t *restrict pwm_signalB __attribute__((aligned(16)));

    /** @brief a shader file to render on the GPU */
    char *shader_file;

    /** 
     * @brief the pwm mapping function to use
     * @see map_byte_image_to_pwm
     * @see map_byte_image_to_pwm_dithered
     */
    func_pwm_mapper_t pwm_mapper;

	/** 
     * @brief the tone mapping function to use, if null no tone mapping applied
     * @see aces_tone_map
     */
    func_tone_mapper_t tone_mapper;

	/** 
     * @brief the tone mapping function to use, if null no tone mapping applied
     * @see aces_tone_map
     */
    func_image_mapper_t image_mapper;

    /**
     * @brief  the target frame rate:
     * maximum frame rate is: 9600 / bpp / (panel_width / 16)
     */
    uint16_t fps;

	/**
     * @brief gamma correction value to use for pwm scaling. if 0 - no gamma is applied
     */
	float gamma;

    bool invert;
    bool image_mirror;
    bool jitter_brightness;

    uint8_t motion_blur_frames;

    float red_gamma;
    float green_gamma;
    float blue_gamma;
    Normal red_linear;
    Normal green_linear;
    Normal blue_linear;

} scene_info;



/**
 * @brief map an image of RGB or RGBA pixels to a pwm signal
 * handles double buffering and tone mapping for you
 * 
 * @param image pointer to the image data
 * @param scene scene configuration
 * @param do_fps_sync set to true to sync to scene->fps based on current time
 */
void map_byte_image_to_pwm(uint8_t *restrict image, const scene_info *scene, const uint8_t do_fps_sync);

/**
 * @brief map an image of RGB or RGBA pixels to a pwm signal with dithering
 * dithering is based on color error diffusion from full 32bpp down to our 
 * target bit depth (usually 15bpp for 32 pwm bits).
 * 
 * dithering 
 * 
 * @param image pointer to the image data
 * @param scene scene configuration
 * @param do_fps_sync set to true to sync to scene->fps based on current time
 */
void map_byte_image_to_pwm_dithered(uint8_t *image, const scene_info *scene, const uint8_t do_fps_sync);
void aces_inplace(RGB *in);
Normal normalize8(const uint8_t in);

void aces_tone_mapper(const RGB *in, RGB *out);
void aces_tone_mapperF(const RGBF *__restrict__ in, RGBF *__restrict__ out);
void hable_tone_mapper(const RGB *in, RGB *out);
void hable_tone_mapperF(const RGBF *in, RGBF *out);
void copy_tone_mapperF(const RGBF *in, RGBF *out);
void reinhard_tone_mapper(const RGB *in, RGB *out);
void reinhard_tone_mapperF(const RGBF *in, RGBF *out);

Normal hable_tone_map(const Normal color);
void habble_inplace(RGB *in);
void adjust_contrast_saturation_inplace(RGBF *__restrict__ in, const float contrast, const float saturation);
uint8_t* u_mapper(const uint8_t *image, uint8_t *output_image, const scene_info *scene);


/**
 * @brief linear interpolation between two floats 
 * 
 * @param float x  value 1
 * @param float y  value 2
 * @param float a  amount to interpolate between (0-1)
 */
float mixf(const float x, const float y, const Normal a);


void draw_square(uint8_t *image, const uint16_t img_width, const uint16_t img_height, const uint8_t stride);
void calculate_fps(long sleep_time);
void *render_shader(void *arg);
void dither_image(uint8_t *image, int width, int height);
void apply_noise_dithering(uint8_t *image, int width, int height);
void check_scene(const scene_info *scene);

/**
 * @brief render the PWM signal to the GPIO pins forever...
 * 
 * @param scene 
 */
__attribute__((noreturn))
void render_forever(const scene_info *scene);

#endif