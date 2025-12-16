#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define PLATFORM_IMPLEMENTATION
#include "plat.h"

#define PNGL_IMPLEMENTATION
#define PNGL_WRITE_IMPLEMENTATION
#include "pngl.h"

#define ARG_PARSE_IMPLEMENTATION

#define ArgParseState ArgParseStateData
#define State_t ArgParseStateData

#include "args.h"

#ifndef M_PI
#define M_PI 3.14159265359f
#endif

struct ArgParseState {
    int dummy;
};

#define TRANSPARENT_MODE_OFF 0
#define TRANSPARENT_MODE_PURE_BLACK 1
#define TRANSPARENT_MODE_SDF_FALLOFF 2

typedef struct {
    float x, y, z;
} vec3;

static vec3 vec3_normalize(vec3 v) {
    float len = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len > 0.0001f) {
        v.x /= len;
        v.y /= len;
        v.z /= len;
    }
    return v;
}

static float vec3_dot(vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

typedef struct {
    float x;
    float y;
} center_coord;

typedef struct {
    float x;
    float y;
    float max_dist;
    int label;
} shape_data;

#define RENDER_MODE_ORIGINAL     0x0001
#define RENDER_MODE_SDF          0x0002
#define RENDER_MODE_NORMAL       0x0004
#define RENDER_MODE_LIT          0x0008
#define RENDER_MODE_MASK         0x0010
#define RENDER_MODE_MAX_DIST     0x0020
#define RENDER_MODE_LOCAL_MAX    0x0040
#define RENDER_MODE_GRADIENT     0x0080
#define RENDER_MODE_SOLID_BG     0x0100

#define RENDER_OVERLAY_POINTS    0x1000
#define RENDER_MODE_NONE         0x0000

typedef struct {
    const char *input_filename;
    const char *output_filename;
    int scale_factor;
    float black_threshold;
    float quality_boost;
    
    int invert_mode;
    float blur_radius;
    const char *blur_type; 
    int kawase_passes; 
    int bw_preprocess_enable;
    
    const char *sdf_algorithm;
    
    unsigned int render_flags;
    const char *point_blending_mode;
    int base_transparent_mode;
    
    const char *yaml_output_filename;
    
    float center_bias[2];
    
    float color_start[3];
    float color_end[3];
    float bg_color[3];
    float gamma;
    
    float pbr_light_intensity;
    vec3 light_dir;
    float roughness;
    float z_scale;
    
    int show_center_points;
    const char *center_method;
    int center_marker_size;
    const char *center_marker_shape;
    float center_marker_color[3];
    
} config;

static config G_CONFIG = {
    NULL,
    "sdf_output.png",
    1,
    10.0f,
    1.0f,
    
    0,
    0.0f,
    "box-blur", 
    2, 
    0,
    
    "fsm",
    
    RENDER_MODE_SDF,
    "normal",
    TRANSPARENT_MODE_OFF,
    
    NULL,
    
    {0.0f, 0.0f},
    
    {0.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 1.0f},
    {0.0f, 0.0f, 0.0f},
    1.0f,
    
    1.0f,
    {0.5f, 0.7f, -0.5f},
    0.3f,
    10.0f,
    
    0,
    "max-dist",
    1,
    "pixel",
    {1.0f, 0.0f, 0.0f},
    
};

static int handle_input(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    if (!argparse_parse_string(argv, argc, i_ptr, &G_CONFIG.input_filename, "input-file")) {
        return 0;
    }
    return 1;
}

static int handle_output(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    if (!argparse_parse_string(argv, argc, i_ptr, &G_CONFIG.output_filename, "output-file")) {
        return 0;
    }
    return 1;
}

static int handle_scale(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    float scale;
    if (!argparse_parse_float(argv, argc, i_ptr, &scale, "scale")) {
        return 0;
    }
    G_CONFIG.scale_factor = (int)scale;
    if (G_CONFIG.scale_factor < 1) {
        G_CONFIG.scale_factor = 1;
    }
    return 1;
}

static int handle_threshold(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    if (!argparse_parse_float(argv, argc, i_ptr, &G_CONFIG.black_threshold, "threshold")) {
        return 0;
    }
    if (G_CONFIG.black_threshold < 0.0f || G_CONFIG.black_threshold > 255.0f) {
        fprintf(stderr, "ERROR: Threshold must be between 0.0 and 255.0.\n");
        return 0;
    }
    return 1;
}

static int handle_quality(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    if (!argparse_parse_float(argv, argc, i_ptr, &G_CONFIG.quality_boost, "quality")) {
        return 0;
    }
    if (G_CONFIG.quality_boost <= 0.0f) {
        fprintf(stderr, "ERROR: Quality boost must be greater than 0.0.\n");
        return 0;
    }
    return 1;
}

static int handle_invert(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    G_CONFIG.invert_mode = 1;
    return 1;
}

static int handle_blur(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    if (!argparse_parse_float(argv, argc, i_ptr, &G_CONFIG.blur_radius, "blur")) {
        return 0;
    }
    if (G_CONFIG.blur_radius < 0.0f) {
        G_CONFIG.blur_radius = 0.0f;
    }
    return 1;
}

static int handle_blur_type(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    const char *type;
    if (!argparse_parse_string(argv, argc, i_ptr, &type, "blur-type")) {
        return 0;
    }
    if (strcmp(type, "box-blur") != 0 && strcmp(type, "kawase") != 0) {
        fprintf(stderr, "ERROR: Blur type must be 'box-blur' or 'kawase'.\n");
        return 0;
    }
    G_CONFIG.blur_type = type;
    return 1;
}

static int handle_kawase_passes(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    float passes;
    if (!argparse_parse_float(argv, argc, i_ptr, &passes, "kawase-passes")) {
        return 0;
    }
    G_CONFIG.kawase_passes = (int)passes;
    if (G_CONFIG.kawase_passes < 1) {
        G_CONFIG.kawase_passes = 1;
    }
    return 1;
}

static int handle_bw_preprocess(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    G_CONFIG.bw_preprocess_enable = 1;
    return 1;
}

static int handle_sdf_algorithm(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    const char *algo;
    if (!argparse_parse_string(argv, argc, i_ptr, &algo, "sdf-algo")) {
        return 0;
    }
    if (strcmp(algo, "fsm") != 0 && strcmp(algo, "mp-fsm") != 0 && strcmp(algo, "jfa") != 0 && strcmp(algo, "brute-force") != 0) {
        fprintf(stderr, "ERROR: SDF algorithm must be 'fsm', 'mp-fsm', 'jfa', or 'brute-force'.\n");
        return 0;
    }
    G_CONFIG.sdf_algorithm = algo;
    return 1;
}

static int handle_base_output_mode(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    const char *mode;
    if (!argparse_parse_string(argv, argc, i_ptr, &mode, "base-output-mode")) {
        return 0;
    }
    
    G_CONFIG.render_flags &= ~0x0FFF;
    
    if (strcmp(mode, "sdf") == 0) G_CONFIG.render_flags |= RENDER_MODE_SDF;
    else if (strcmp(mode, "solid-bg") == 0) G_CONFIG.render_flags |= RENDER_MODE_SOLID_BG;
    else if (strcmp(mode, "original") == 0) G_CONFIG.render_flags |= RENDER_MODE_ORIGINAL;
    else if (strcmp(mode, "normal") == 0) G_CONFIG.render_flags |= RENDER_MODE_NORMAL;
    else if (strcmp(mode, "lit") == 0) G_CONFIG.render_flags |= RENDER_MODE_LIT;
    else if (strcmp(mode, "mask") == 0) G_CONFIG.render_flags |= RENDER_MODE_MASK;
    else if (strcmp(mode, "max-dist") == 0) G_CONFIG.render_flags |= RENDER_MODE_MAX_DIST;
    else if (strcmp(mode, "local-max") == 0) G_CONFIG.render_flags |= RENDER_MODE_LOCAL_MAX;
    else if (strcmp(mode, "gradient") == 0) G_CONFIG.render_flags |= RENDER_MODE_GRADIENT;
    else {
        fprintf(stderr, "ERROR: Invalid base output mode: %s\n", mode);
        return 0;
    }

    return 1;
}

static int handle_yaml_output(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    if (!argparse_parse_string(argv, argc, i_ptr, &G_CONFIG.yaml_output_filename, "yaml-output")) {
        return 0;
    }
    return 1;
}

static int handle_center_bias(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    float x_bias, y_bias;
    int i = *i_ptr;
    
    if (i + 2 >= argc) {
        fprintf(stderr, "ERROR: Missing X, Y values for --center-bias. Expected 2 floats.\n");
        return 0;
    }
    
    if (!argparse_parse_float(argv, argc, i_ptr, &x_bias, "center-bias X") ||
        !argparse_parse_float(argv, argc, i_ptr, &y_bias, "center-bias Y")) {
        return 0;
    }

    G_CONFIG.center_bias[0] = x_bias;
    G_CONFIG.center_bias[1] = y_bias;
    return 1;
}

static int handle_point_blending_mode(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    const char *mode;
    if (!argparse_parse_string(argv, argc, i_ptr, &mode, "point-blend-mode")) {
        return 0;
    }
    if (strcmp(mode, "normal") != 0 && strcmp(mode, "and") != 0 &&
        strcmp(mode, "or") != 0 && strcmp(mode, "xor") != 0) {
        fprintf(stderr, "ERROR: Point blend mode must be 'normal', 'and', 'or', or 'xor'.\n");
        return 0;
    }
    G_CONFIG.point_blending_mode = mode;
    return 1;
}

static int handle_color_start(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    float r, g, b;
    int i = *i_ptr;
    
    if (i + 3 >= argc) {
        fprintf(stderr, "ERROR: Missing R, G, B values for --color-start. Expected 3 floats.\n");
        return 0;
    }
    
    if (!argparse_parse_float(argv, argc, i_ptr, &r, "color-start R") ||
        !argparse_parse_float(argv, argc, i_ptr, &g, "color-start G") ||
        !argparse_parse_float(argv, argc, i_ptr, &b, "color-start B")) {
        return 0;
    }

    G_CONFIG.color_start[0] = r;
    G_CONFIG.color_start[1] = g;
    G_CONFIG.color_start[2] = b;
    return 1;
}

static int handle_color_end(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    float r, g, b;
    int i = *i_ptr;
    
    if (i + 3 >= argc) {
        fprintf(stderr, "ERROR: Missing R, G, B values for --color-end. Expected 3 floats.\n");
        return 0;
    }
    
    if (!argparse_parse_float(argv, argc, i_ptr, &r, "color-end R") ||
        !argparse_parse_float(argv, argc, i_ptr, &g, "color-end G") ||
        !argparse_parse_float(argv, argc, i_ptr, &b, "color-end B")) {
        return 0;
    }
    
    G_CONFIG.color_end[0] = r;
    G_CONFIG.color_end[1] = g;
    G_CONFIG.color_end[2] = b;
    return 1;
}

static int handle_bg_color(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    float r, g, b;
    int i = *i_ptr;
    
    if (i + 3 >= argc) {
        fprintf(stderr, "ERROR: Missing R, G, B values for --bg-color. Expected 3 floats.\n");
        return 0;
    }
    
    if (!argparse_parse_float(argv, argc, i_ptr, &r, "bg-color R") ||
        !argparse_parse_float(argv, argc, i_ptr, &g, "bg-color G") ||
        !argparse_parse_float(argv, argc, i_ptr, &b, "bg-color B")) {
        return 0;
    }
    
    G_CONFIG.bg_color[0] = r;
    G_CONFIG.bg_color[1] = g;
    G_CONFIG.bg_color[2] = b;
    return 1;
}

static int handle_gamma(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    if (!argparse_parse_float(argv, argc, i_ptr, &G_CONFIG.gamma, "gamma")) {
        return 0;
    }
    if (G_CONFIG.gamma <= 0.0f) {
        fprintf(stderr, "ERROR: Gamma must be greater than 0.0.\n");
        return 0;
    }
    return 1;
}

static int handle_lighting_intensity(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    float intensity;
    G_CONFIG.pbr_light_intensity = 1.0f;
		if (!argparse_parse_float(argv, argc, i_ptr, &intensity, "light-intensity")) {
        return 0;
    }
    G_CONFIG.pbr_light_intensity = intensity;
    return 1;
}

static int handle_light_dir(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    float x, y, z;
    int i = *i_ptr;

    if (i + 3 >= argc) {
        fprintf(stderr, "ERROR: Missing X, Y, Z values for --light-dir. Expected 3 floats.\n");
        return 0;
    }

    if (!argparse_parse_float(argv, argc, i_ptr, &x, "light-dir X") ||
        !argparse_parse_float(argv, argc, i_ptr, &y, "light-dir Y") ||
        !argparse_parse_float(argv, argc, i_ptr, &z, "light-dir Z")) {
        return 0;
    }
    G_CONFIG.light_dir.x = x;
    G_CONFIG.light_dir.y = y;
    G_CONFIG.light_dir.z = z;
    return 1;
}

static int handle_roughness(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    if (!argparse_parse_float(argv, argc, i_ptr, &G_CONFIG.roughness, "roughness")) {
        return 0;
    }
    if (G_CONFIG.roughness < 0.0f || G_CONFIG.roughness > 1.0f) {
        fprintf(stderr, "ERROR: Roughness must be between 0.0 and 1.0.\n");
        return 0;
    }
    return 1;
}

static int handle_z_scale(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    if (!argparse_parse_float(argv, argc, i_ptr, &G_CONFIG.z_scale, "z-scale")) {
        return 0;
    }
    if (G_CONFIG.z_scale <= 0.0f) {
        fprintf(stderr, "ERROR: Z-Scale must be greater than 0.0.\n");
        return 0;
    }
    return 1;
}

static int handle_center_points(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    G_CONFIG.show_center_points = 1;
    return 1;
}

static int handle_center_method(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    const char *method;
    if (!argparse_parse_string(argv, argc, i_ptr, &method, "center-method")) {
        return 0;
    }
    if (strcmp(method, "max-dist") != 0 && strcmp(method, "centroid") != 0) {
        fprintf(stderr, "ERROR: Center method must be 'max-dist' or 'centroid'.\n");
        return 0;
    }
    G_CONFIG.center_method = method;
    return 1;
}

static int handle_marker_size(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    float size;
    if (!argparse_parse_float(argv, argc, i_ptr, &size, "marker-size")) {
        return 0;
    }
    G_CONFIG.center_marker_size = (int)size;
    if (G_CONFIG.center_marker_size < 1) {
        G_CONFIG.center_marker_size = 1;
    }
    return 1;
}

static int handle_marker_shape(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    const char *shape;
    if (!argparse_parse_string(argv, argc, i_ptr, &shape, "marker-shape")) {
        return 0;
    }
    if (strcmp(shape, "cross") != 0 && strcmp(shape, "pixel") != 0) {
        fprintf(stderr, "ERROR: Marker shape must be 'cross' or 'pixel'.\n");
        return 0;
    }
    G_CONFIG.center_marker_shape = shape;
    return 1;
}

static int handle_marker_color(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    float r, g, b;
    int i = *i_ptr;

    if (i + 3 >= argc) {
        fprintf(stderr, "ERROR: Missing R, G, B values for --marker-color. Expected 3 floats.\n");
        return 0;
    }

    if (!argparse_parse_float(argv, argc, i_ptr, &r, "marker-color R") ||
        !argparse_parse_float(argv, argc, i_ptr, &g, "marker-color G") ||
        !argparse_parse_float(argv, argc, i_ptr, &b, "marker-color B")) {
        return 0;
    }

    G_CONFIG.center_marker_color[0] = r;
    G_CONFIG.center_marker_color[1] = g;
    G_CONFIG.center_marker_color[2] = b;
    return 1;
}

static int handle_transparent_black(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    const char *mode = NULL;
    int i = *i_ptr;
    
    if (i + 1 < argc && argv[i+1][0] != '-') {
        if (!argparse_parse_string(argv, argc, i_ptr, &mode, "transparent-black mode")) {
            return 0;
        }
    } else {
        mode = "pure";
    }

    if (mode) {
        if (strcmp(mode, "pure") == 0) {
            G_CONFIG.base_transparent_mode = TRANSPARENT_MODE_PURE_BLACK;
        } else if (strcmp(mode, "smooth") == 0) {
            G_CONFIG.base_transparent_mode = TRANSPARENT_MODE_SDF_FALLOFF;
        } else {
            fprintf(stderr, "ERROR: --transparent-black mode must be 'pure' or 'smooth'.\n");
            return 0;
        }
    } else {
        G_CONFIG.base_transparent_mode = TRANSPARENT_MODE_PURE_BLACK;
    }
    return 1;
}

static int handle_help(struct ArgParseState *state, char **argv, int argc, int *i_ptr) {
    return -1;
}

static ArgOption G_OPTIONS[] = {
    {"-i", "--input", handle_input, "Input image file path (PNG). <val>", 1, 0},
    {"-o", "--output", handle_output, "Output image file path (PNG). <val>", 0, 0},
    {"-s", "--scale", handle_scale, "Output resolution scale factor (e.g., 2.0). <val>", 0, 0},
    {"-t", "--threshold", handle_threshold, "Black/boundary luminance threshold (0-255). <val>", 0, 0},
    {"-q", "--quality", handle_quality, "Distance normalization factor (boost effect). <val>", 0, 0},
    {"-v", "--invert", handle_invert, "Invert image before processing (white shape, black background).", 0, 0},
    {"-b", "--blur", handle_blur, "Blur radius (0.0 to N, e.g., 1.5). <val>", 0, 0},
    {"-bt", "--blur-type", handle_blur_type, "Blur algorithm: 'box-blur' (default) or 'kawase'. <val>", 0, 0},
    {"-kp", "--kawase-passes", handle_kawase_passes, "Number of passes for Kawase blur (min 1). <val>", 0, 0},
    {"-bw", "--bw-preprocess", handle_bw_preprocess, "Preprocess image to black and white using threshold.", 0, 0},
    {"-sa", "--sdf-algo", handle_sdf_algorithm, "SDF Algorithm: 'fsm' (Fast), 'mp-fsm' (Multi-Pass, Quality), 'jfa' (Jump Flooding, Fast/High-Quality), or 'brute-force' (Slowest, Best). <val>", 0, 0},
    {"-om", "--base-output-mode", handle_base_output_mode, "Base image output mode: 'sdf', 'solid-bg', 'original', 'lit', 'normal', 'mask', 'max-dist', 'local-max', or 'gradient'. (Default: sdf). <val>", 0, 0},
    {"-y", "--yaml-output", handle_yaml_output, "Output center point data to a YAML file. <val>", 0, 0},
    {"-cb", "--center-bias", handle_center_bias, "Bias center position (X, Y) in pixels. <X> <Y>", 0, 0},
    {"-pbm", "--point-blend-mode", handle_point_blending_mode, "Point blending mode: 'normal', 'and', 'or', or 'xor'. <val>", 0, 0},
    {"-cs", "--color-start", handle_color_start, "RGB for boundary (0.0-1.0), e.g., 1.0 0.0 0.0. <R> <G> <B>", 0, 0},
    {"-ce", "--color-end", handle_color_end, "RGB for furthest distance (0.0-1.0), e.g., 0.0 1.0 0.0. <R> <G> <B>", 0, 0},
    {"-bg", "--bg-color", handle_bg_color, "Background RGB color for solid/SDF modes (0.0-1.0). <R> <G> <B>", 0, 0},
    {"-g", "--gamma", handle_gamma, "Gamma correction value (e.g., 2.2). <val>", 0, 0},
    {"-l", "--light-intensity", handle_lighting_intensity, "PBR direct light intensity multiplier (default 1.0). <val>", 0, 0},
    {"-ld", "--light-dir", handle_light_dir, "Light direction (X, Y, Z). Defaults to {0.5, 0.7, -0.5}. <X> <Y> <Z>", 0, 0},
    {"-r", "--roughness", handle_roughness, "PBR Roughness (0.0-1.0). <val>", 0, 0},
    {"-z", "--z-scale", handle_z_scale, "Height exaggeration for normal calculation. <val>", 0, 0},
    {"-cp", "--center-points", handle_center_points, "Analyze center point of each shape, and apply as an overlay if a base mode is set, or as a stand-alone output otherwise.", 0, 0},
    {"-cm", "--center-method", handle_center_method, "Center method: 'max-dist' (Pole of Inaccessibility) or 'centroid' (Center of Mass). <val>", 0, 0},
    {"-ms", "--marker-size", handle_marker_size, "Marker size/radius (1 for pixel, 2 for 5x5 cross, etc.). <val>", 0, 0},
    {"-msh", "--marker-shape", handle_marker_shape, "Marker shape: 'cross' or 'pixel'. <val>", 0, 0},
    {"-mc", "--marker-color", handle_marker_color, "Marker RGB color (0.0-1.0), e.g., 1.0 1.0 0.0 (Yellow). <R> <G> <B>", 0, 0},
    {"-tb", "--transparent-black", handle_transparent_black, "Output pure black pixels as transparent (RGBA). Optional mode: 'pure' (default) or 'smooth' (SDF depth falloff). <val>", 0, 0},
    {"-h", "--help", handle_help, "Print this help message and exit.", 0, 0},
    {NULL, NULL, NULL, NULL, 0, 0}
};

static void apply_box_blur(pngl_uc *data, int w, int h, int comp, int radius) {
    pngl_uc *temp_data;
    float *running_sum;
    int x, y, c, i, kernel_size;
    
    if (radius <= 0) return;

    kernel_size = 2 * radius + 1;
    temp_data = (pngl_uc *)malloc(w * h * comp * sizeof(pngl_uc));
    running_sum = (float *)malloc(w * comp * sizeof(float));
    
    if (!temp_data || !running_sum) {
        if (temp_data) free(temp_data);
        if (running_sum) free(running_sum);
        return;
    }
    
    for (y = 0; y < h; ++y) {
        
        for (c = 0; c < comp; ++c) {
            running_sum[c] = 0.0f;
            for (i = -radius; i <= radius; ++i) {
                int x_tap = (i < 0) ? 0 : ((i >= w) ? w - 1 : i);
                running_sum[c] += data[(y * w + x_tap) * comp + c];
            }
            temp_data[(y * w + 0) * comp + c] = (pngl_uc)(running_sum[c] / (float)kernel_size);
        }
        
        for (x = 1; x < w; ++x) {
            for (c = 0; c < comp; ++c) {
                int index = (y * w + x) * comp + c;
                int idx_left = (y * w + x - radius - 1) * comp + c;
                int idx_right = (y * w + x + radius) * comp + c;
                
                int x_tap_left = x - radius - 1;
                int x_tap_right = x + radius;
                
                float val_left = (x_tap_left < 0) ? data[(y * w + 0) * comp + c] : data[idx_left];
                float val_right = (x_tap_right >= w) ? data[(y * w + w - 1) * comp + c] : data[idx_right];
                
                running_sum[c] += val_right - val_left;
                temp_data[index] = (pngl_uc)(running_sum[c] / (float)kernel_size);
            }
        }
    }
    
    for (x = 0; x < w; ++x) {
        
        for (c = 0; c < comp; ++c) {
            running_sum[c] = 0.0f;
            for (i = -radius; i <= radius; ++i) {
                int y_tap = (i < 0) ? 0 : ((i >= h) ? h - 1 : i);
                running_sum[c] += temp_data[(y_tap * w + x) * comp + c];
            }
            data[(0 * w + x) * comp + c] = (pngl_uc)(running_sum[c] / (float)kernel_size);
        }
        
        for (y = 1; y < h; ++y) {
            for (c = 0; c < comp; ++c) {
                int index = (y * w + x) * comp + c;
                int idx_top = ((y - radius - 1) * w + x) * comp + c;
                int idx_bottom = ((y + radius) * w + x) * comp + c;
                
                int y_tap_top = y - radius - 1;
                int y_tap_bottom = y + radius;

                float val_top = (y_tap_top < 0) ? temp_data[(0 * w + x) * comp + c] : temp_data[idx_top];
                float val_bottom = (y_tap_bottom >= h) ? temp_data[((h - 1) * w + x) * comp + c] : temp_data[idx_bottom];
                
                running_sum[c] += val_bottom - val_top;
                data[index] = (pngl_uc)(running_sum[c] / (float)kernel_size);
            }
        }
    }

    free(temp_data);
    free(running_sum);
}

static void apply_kawase_blur(pngl_uc *data, int w, int h, int comp, int passes, float base_radius) {
    pngl_uc *temp_data;
    int p, x, y, c, i;
    int buffer_size = w * h * comp * sizeof(pngl_uc);
    
    if (passes < 1 || base_radius <= 0.0f) return;

    temp_data = (pngl_uc *)malloc(buffer_size);
    if (!temp_data) {
        return;
    }

    for (p = 0; p < passes; ++p) {
        int offset = (int)base_radius + p;
        
        pngl_uc *input = (p % 2 == 0) ? data : temp_data;
        pngl_uc *output = (p % 2 == 0) ? temp_data : data;

        if (p > 0) {
             if (p % 2 != 0) {
                memcpy(temp_data, data, buffer_size);
             } else {
                memcpy(data, temp_data, buffer_size);
             }
        }
        
        for (y = 0; y < h; ++y) {
            for (x = 0; x < w; ++x) {
                int index = y * w + x;
                float sum[4] = {0.0f, 0.0f, 0.0f, 0.0f}; 

                int offsets[4][2] = {
                    {-offset, -offset}, 
                    { offset, -offset}, 
                    {-offset,  offset}, 
                    { offset,  offset}
                };

                for (i = 0; i < 4; ++i) {
                    int nx = x + offsets[i][0];
                    int ny = y + offsets[i][1];

                    if (nx < 0) nx = 0;
                    if (nx >= w) nx = w - 1;
                    if (ny < 0) ny = 0;
                    if (ny >= h) ny = h - 1;
                    
                    int n_idx = (ny * w + nx) * comp;

                    for (c = 0; c < comp; ++c) {
                         sum[c] += input[n_idx + c];
                    }
                }
                
                int center_idx = index * comp;
                for (c = 0; c < comp; ++c) {
                    output[center_idx + c] = (pngl_uc)(sum[c] / 4.0f);
                }
            }
        }
    }
    
    if (passes % 2 != 0) {
        memcpy(data, temp_data, buffer_size);
    }
    
    free(temp_data);
}


static void apply_bw_threshold(pngl_uc *data, int w, int h, int comp) {
    int x, y;
    int threshold_val = (int)G_CONFIG.black_threshold;
    
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            long index = (long)y * w + x;
            int r, g, b, avg;
            
            if (comp < 3) {
                if (data[index * comp] <= threshold_val) {
                    data[index * comp] = 0;
                } else {
                    data[index * comp] = 255;
                }
                continue;
            }
            
            r = data[index * comp + 0];
            g = data[index * comp + 1];
            b = data[index * comp + 2];
            
            avg = (r + g + b) / 3;
            
            if (avg <= threshold_val) {
                data[index * comp + 0] = 0;
                data[index * comp + 1] = 0;
                data[index * comp + 2] = 0;
            } else {
                data[index * comp + 0] = 255;
                data[index * comp + 1] = 255;
                data[index * comp + 2] = 255;
            }
        }
    }
}

static void preprocess_image(pngl_uc *data, int w, int h, int comp) {
    int i, total_pixels = w * h;
    int radius = (int)G_CONFIG.blur_radius;

    if (G_CONFIG.bw_preprocess_enable) {
        apply_bw_threshold(data, w, h, comp);
    }

    if (radius > 0) {
        if (strcmp(G_CONFIG.blur_type, "kawase") == 0) {
            apply_kawase_blur(data, w, h, comp, G_CONFIG.kawase_passes, G_CONFIG.blur_radius);
        } else {
            apply_box_blur(data, w, h, comp, radius);
        }
    }

    if (G_CONFIG.invert_mode) {
        for (i = 0; i < total_pixels * comp; ++i) {
            data[i] = 255 - data[i];
        }
    }
}

static int get_pixel_luminance(const pngl_uc *data, int x, int y, int w, int comp) {
    long r, g, b;
    int index = (y * w + x) * comp;
    int black_threshold = (int)G_CONFIG.black_threshold;

    if (comp >= 3) {
        r = data[index + 0];
        g = data[index + 1];
        b = data[index + 2];

        if (r <= black_threshold && g <= black_threshold && b <= black_threshold) {
            return 0;
        }
    } else if (comp == 1) {
        if (data[index] <= black_threshold) {
            return 0;
        }
    }
    return 255;
}

typedef struct {
    float x, y;
} coord;

static float dist_sq(float x1, float y1, float x2, float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return dx * dx + dy * dy;
}

static float *generate_sdf_fsm_2_pass(const pngl_uc *img_data, int w_in, int h_in, int comp, coord *closest_coord) {
    int w_out = w_in * G_CONFIG.scale_factor;
    int h_out = h_in * G_CONFIG.scale_factor;
    float max_dist_sq = 0.0f;
    float *float_dist_map;
    float *dist_sq_map;
    int x, y, i;
    int max_coord_val = w_out + h_out + 1;
    float max_coord_val_sq;
    float dist_sq_neighbor;
    
    max_coord_val_sq = (float)max_coord_val * max_coord_val;
    
    float_dist_map = (float *)malloc(w_out * h_out * sizeof(float));
    dist_sq_map = (float *)malloc(w_out * h_out * sizeof(float));
    
    if (!float_dist_map || !dist_sq_map) {
        if (float_dist_map) free(float_dist_map);
        if (dist_sq_map) free(dist_sq_map);
        return NULL;
    }

    for (y = 0; y < h_out; ++y) {
        for (x = 0; x < w_out; ++x) {
            i = y * w_out + x;
            dist_sq_map[i] = max_coord_val_sq;
            
            {
                int bx = x / G_CONFIG.scale_factor;
                int by = y / G_CONFIG.scale_factor;
                if (get_pixel_luminance(img_data, bx, by, w_in, comp) == 0) {
                    closest_coord[i].x = (float)x;
                    closest_coord[i].y = (float)y;
                    dist_sq_map[i] = 0.0f;
                    continue;
                }
            }

            closest_coord[i].x = (float)max_coord_val;
            closest_coord[i].y = (float)max_coord_val;

            
            if (x > 0) {
                coord c_left = closest_coord[i - 1];
                dist_sq_neighbor = dist_sq((float)x, (float)y, c_left.x, c_left.y);

                if (dist_sq_neighbor < dist_sq_map[i]) {
                    closest_coord[i] = c_left;
                    dist_sq_map[i] = dist_sq_neighbor;
                }
            }
            
            if (y > 0) {
                coord c_top = closest_coord[i - w_out];
                dist_sq_neighbor = dist_sq((float)x, (float)y, c_top.x, c_top.y);
                
                if (dist_sq_neighbor < dist_sq_map[i]) {
                    closest_coord[i] = c_top;
                    dist_sq_map[i] = dist_sq_neighbor;
                }
            }
        }
    }

    for (y = h_out - 1; y >= 0; --y) {
        for (x = w_out - 1; x >= 0; --x) {
            i = y * w_out + x;

            if (x < w_out - 1) {
                coord c_right = closest_coord[i + 1];
                dist_sq_neighbor = dist_sq((float)x, (float)y, c_right.x, c_right.y);

                if (dist_sq_neighbor < dist_sq_map[i]) {
                    closest_coord[i] = c_right;
                    dist_sq_map[i] = dist_sq_neighbor;
                }
            }
            
            if (y < h_out - 1) {
                coord c_bottom = closest_coord[i + w_out];
                dist_sq_neighbor = dist_sq((float)x, (float)y, c_bottom.x, c_bottom.y);
                
                if (dist_sq_neighbor < dist_sq_map[i]) {
                    closest_coord[i] = c_bottom;
                    dist_sq_map[i] = dist_sq_neighbor;
                }
            }
            
            if (dist_sq_map[i] > max_dist_sq) {
                max_dist_sq = dist_sq_map[i];
            }
        }
    }

    if (max_dist_sq == 0.0f) {
        max_dist_sq = 1.0f;
    }

    {
        float inv_max_dist = 1.0f / sqrt(max_dist_sq);
        for (i = 0; i < w_out * h_out; ++i) {
            float_dist_map[i] = sqrt(dist_sq_map[i]) * inv_max_dist;
        }
    }
    
    free(dist_sq_map);
    return float_dist_map;
}

static float *generate_sdf_fsm_multi_pass(const pngl_uc *img_data, int w_in, int h_in, int comp, coord *closest_coord) {
    int w_out = w_in * G_CONFIG.scale_factor;
    int h_out = h_in * G_CONFIG.scale_factor;
    float max_dist_sq = 0.0f;
    float *float_dist_map;
    float *dist_sq_map;
    int x, y, i, pass;
    int max_coord_val = w_out + h_out + 1;
    float max_coord_val_sq;
    float dist_sq_neighbor;

    max_coord_val_sq = (float)max_coord_val * max_coord_val;
    
    float_dist_map = (float *)malloc(w_out * h_out * sizeof(float));
    dist_sq_map = (float *)malloc(w_out * h_out * sizeof(float));
    
    if (!float_dist_map || !dist_sq_map) {
        if (float_dist_map) free(float_dist_map);
        if (dist_sq_map) free(dist_sq_map);
        return NULL;
    }
    
    for (y = 0; y < h_out; ++y) {
        for (x = 0; x < w_out; ++x) {
            i = y * w_out + x;
            dist_sq_map[i] = max_coord_val_sq;
            
            {
                int bx = x / G_CONFIG.scale_factor;
                int by = y / G_CONFIG.scale_factor;
                if (get_pixel_luminance(img_data, bx, by, w_in, comp) == 0) {
                    closest_coord[i].x = (float)x;
                    closest_coord[i].y = (float)y;
                    dist_sq_map[i] = 0.0f;
                    continue;
                }
            }

            closest_coord[i].x = (float)max_coord_val;
            closest_coord[i].y = (float)max_coord_val;

            if (x > 0) {
                coord c_left = closest_coord[i - 1];
                dist_sq_neighbor = dist_sq((float)x, (float)y, c_left.x, c_left.y);

                if (dist_sq_neighbor < dist_sq_map[i]) {
                    closest_coord[i] = c_left;
                    dist_sq_map[i] = dist_sq_neighbor;
                }
            }
            
            if (y > 0) {
                coord c_top = closest_coord[i - w_out];
                dist_sq_neighbor = dist_sq((float)x, (float)y, c_top.x, c_top.y);
                
                if (dist_sq_neighbor < dist_sq_map[i]) {
                    closest_coord[i] = c_top;
                    dist_sq_map[i] = dist_sq_neighbor;
                }
            }
        }
    }

    for (pass = 0; pass < 4; ++pass) {
        for (y = h_out - 1; y >= 0; --y) {
            for (x = w_out - 1; x >= 0; --x) {
                i = y * w_out + x;
                
                if (x < w_out - 1) {
                    coord c_right = closest_coord[i + 1];
                    dist_sq_neighbor = dist_sq((float)x, (float)y, c_right.x, c_right.y);

                    if (dist_sq_neighbor < dist_sq_map[i]) {
                        closest_coord[i] = c_right;
                        dist_sq_map[i] = dist_sq_neighbor;
                    }
                }
                
                if (y < h_out - 1) {
                    coord c_bottom = closest_coord[i + w_out];
                    dist_sq_neighbor = dist_sq((float)x, (float)y, c_bottom.x, c_bottom.y);
                    
                    if (dist_sq_neighbor < dist_sq_map[i]) {
                        closest_coord[i] = c_bottom;
                        dist_sq_map[i] = dist_sq_neighbor;
                    }
                }
            }
        }
        
        for (y = 0; y < h_out; ++y) {
            for (x = 0; x < w_out; ++x) {
                i = y * w_out + x;
                
                if (x > 0) {
                    coord c_left = closest_coord[i - 1];
                    dist_sq_neighbor = dist_sq((float)x, (float)y, c_left.x, c_left.y);

                    if (dist_sq_neighbor < dist_sq_map[i]) {
                        closest_coord[i] = c_left;
                        dist_sq_map[i] = dist_sq_neighbor;
                    }
                }
                
                if (y > 0) {
                    coord c_top = closest_coord[i - w_out];
                    dist_sq_neighbor = dist_sq((float)x, (float)y, c_top.x, c_top.y);
                    
                    if (dist_sq_neighbor < dist_sq_map[i]) {
                        closest_coord[i] = c_top;
                        dist_sq_map[i] = dist_sq_neighbor;
                    }
                }
            }
        }
    }

    for (i = 0; i < w_out * h_out; ++i) {
        if (dist_sq_map[i] > max_dist_sq) {
            max_dist_sq = dist_sq_map[i];
        }
    }

    if (max_dist_sq == 0.0f) {
        max_dist_sq = 1.0f;
    }

    {
        float inv_max_dist = 1.0f / sqrt(max_dist_sq);
        for (i = 0; i < w_out * h_out; ++i) {
            float_dist_map[i] = sqrt(dist_sq_map[i]) * inv_max_dist;
        }
    }

    free(dist_sq_map);
    return float_dist_map;
}

static float *generate_sdf_jfa(const pngl_uc *img_data, int w_in, int h_in, int comp, coord *closest_coord) {
    int w_out = w_in * G_CONFIG.scale_factor;
    int h_out = h_in * G_CONFIG.scale_factor;
    float max_dist_sq = 0.0f;
    float *float_dist_map;
    int x, y, i, k;
    int max_coord_val = w_out + h_out + 1;
    float max_coord_val_sq;
    float dist_sq_neighbor;
    float dist_sq_current;
    int step;
    int max_dim = w_out > h_out ? w_out : h_out;
    int log_max_dim = (int)ceil(log((double)max_dim) / log(2.0));
    
    max_coord_val_sq = (float)max_coord_val * max_coord_val;

    float_dist_map = (float *)malloc(w_out * h_out * sizeof(float));
    if (!float_dist_map) {
        return NULL;
    }

    for (y = 0; y < h_out; ++y) {
        for (x = 0; x < w_out; ++x) {
            i = y * w_out + x;
            
            {
                int bx = x / G_CONFIG.scale_factor;
                int by = y / G_CONFIG.scale_factor;
                if (get_pixel_luminance(img_data, bx, by, w_in, comp) == 0) {
                    closest_coord[i].x = (float)x;
                    closest_coord[i].y = (float)y;
                    continue;
                }
            }

            closest_coord[i].x = (float)max_coord_val;
            closest_coord[i].y = (float)max_coord_val;
        }
    }
    
    printf("SDF Algo: JFA - Running %d passes...\n", log_max_dim);

    for (step = log_max_dim - 1; step >= 0; --step) {
        k = 1 << step;
        
        for (y = 0; y < h_out; ++y) {
            for (x = 0; x < w_out; ++x) {
                i = y * w_out + x;
                
                int nx, ny;
                int dx, dy;
                float current_closest_x = closest_coord[i].x;
                float current_closest_y = closest_coord[i].y;

                for (dy = -k; dy <= k; dy += k) {
                    for (dx = -k; dx <= k; dx += k) {
                        if (dx == 0 && dy == 0) continue; 
                        
                        nx = x + dx;
                        ny = y + dy;

                        if (nx >= 0 && nx < w_out && ny >= 0 && ny < h_out) {
                            int ni = ny * w_out + nx;
                            coord c_neighbor = closest_coord[ni];
                            
                            if (c_neighbor.x != (float)max_coord_val) {
                                
                                dist_sq_current = dist_sq((float)x, (float)y, current_closest_x, current_closest_y);
                                
                                dist_sq_neighbor = dist_sq((float)x, (float)y, c_neighbor.x, c_neighbor.y);
                                
                                if (dist_sq_neighbor < dist_sq_current) {
                                    closest_coord[i] = c_neighbor;
                                    current_closest_x = c_neighbor.x;
                                    current_closest_y = c_neighbor.y;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    for (i = 0; i < w_out * h_out; ++i) {
        float dx, dy, distance_sq;
        
        dx = (float)(i % w_out) - closest_coord[i].x;
        dy = (float)((int)(i / w_out)) - (closest_coord[i].y);
        distance_sq = dx * dx + dy * dy;
        
        float_dist_map[i] = sqrt(distance_sq);
        
        if (distance_sq > max_dist_sq) {
            max_dist_sq = distance_sq;
        }
    }

    if (max_dist_sq == 0.0f) {
        max_dist_sq = 1.0f;
    }

    {
        float inv_max_dist = 1.0f / sqrt(max_dist_sq);
        for (i = 0; i < w_out * h_out; ++i) {
            float_dist_map[i] *= inv_max_dist;
        }
    }

    return float_dist_map;
}

typedef struct {
    float *dist_map;
    coord *closest_coord;
    const coord *boundary_coords;
    const pngl_uc *img_data;
    int boundary_count;
    int w_out;
    int h_out;
    int w_in;
    int comp_in;
    int max_coord_val;
    int start_y;
    int end_y;
} brute_force_thread_args;


#ifdef _WIN32
static DWORD WINAPI generate_sdf_brute_force_thread(LPVOID param) {
#else
static void *generate_sdf_brute_force_thread(void *param) {
#endif
    brute_force_thread_args *args = (brute_force_thread_args *)param;
    int y, x, i, b_idx;
    float max_coord_val_sq = (float)args->max_coord_val * args->max_coord_val;
    int w_out = args->w_out;
    int w_in = args->w_in;
    int comp_in = args->comp_in;

    for (y = args->start_y; y < args->end_y; ++y) {
        for (x = 0; x < w_out; ++x) {
            i = y * w_out + x;
            
            {
                int bx = x / G_CONFIG.scale_factor;
                int by = y / G_CONFIG.scale_factor;
                if (get_pixel_luminance(args->img_data, bx, by, w_in, comp_in) == 0) {
                    args->closest_coord[i].x = (float)x;
                    args->closest_coord[i].y = (float)y;
                    args->dist_map[i] = 0.0f;
                    continue;
                }
            }
            
            {
                float min_dist_sq = max_coord_val_sq;
                coord current_closest;
                float current_x = (float)x;
                float current_y = (float)y;

                for (b_idx = 0; b_idx < args->boundary_count; ++b_idx) {
                    float dist_sq_current = dist_sq(current_x, current_y, args->boundary_coords[b_idx].x, args->boundary_coords[b_idx].y);
                    
                    if (dist_sq_current < min_dist_sq) {
                        min_dist_sq = dist_sq_current;
                        current_closest = args->boundary_coords[b_idx];
                    }
                }
                
                args->closest_coord[i] = current_closest;
                args->dist_map[i] = sqrt(min_dist_sq);
            }
        }
    }
    
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}


static float *generate_sdf_brute_force(const pngl_uc *img_data, int w_in, int h_in, int comp, coord *closest_coord) {
    int w_out = w_in * G_CONFIG.scale_factor;
    int h_out = h_in * G_CONFIG.scale_factor;
    float max_dist = 0.0f;
    float *float_dist_map = NULL;
    int x, y, i;
    int max_coord_val = w_out + h_out + 1;
    
    coord *boundary_coords;
    int boundary_count = 0;
    int bx, by;
    
    int num_cores = platform_get_num_cpu_cores();
    int num_threads = num_cores > 0 ? num_cores : 1;
    int rows_per_thread, start_y, end_y, thread_i, join_i;
    
    brute_force_thread_args *thread_args;
    
#ifdef _WIN32
    HANDLE *threads;
#else
    pthread_t *threads;
#endif
    
    float_dist_map = (float *)malloc(w_out * h_out * sizeof(float));
    if (!float_dist_map) {
        return NULL;
    }

    for (y = 0; y < h_out; ++y) {
        for (x = 0; x < w_out; ++x) {
            bx = x / G_CONFIG.scale_factor;
            by = y / G_CONFIG.scale_factor;
            if (get_pixel_luminance(img_data, bx, by, w_in, comp) == 0) {
                boundary_count++;
            }
        }
    }

    boundary_coords = (coord *)malloc(boundary_count * sizeof(coord));
    if (!boundary_coords) {
        free(float_dist_map);
        return NULL;
    }
    
    if (boundary_count == 0) {
        free(boundary_coords);
        for (i = 0; i < w_out * h_out; ++i) {
            float_dist_map[i] = 0.0f;
        }
        return float_dist_map;
    }
    
    {
        int current_count = 0;
        for (y = 0; y < h_out; ++y) {
            for (x = 0; x < w_out; ++x) {
                bx = x / G_CONFIG.scale_factor;
                by = y / G_CONFIG.scale_factor;
                if (get_pixel_luminance(img_data, bx, by, w_in, comp) == 0) {
                    boundary_coords[current_count].x = (float)x;
                    boundary_coords[current_count].y = (float)y;
                    current_count++;
                }
            }
        }
    }
    
    printf("SDF Algo: Brute Force (Platform Threaded) - Using %d threads for %d boundary points...\n", num_threads, boundary_count);

#ifdef _WIN32
    threads = (HANDLE *)malloc(num_threads * sizeof(HANDLE));
#else
    threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
#endif
    thread_args = (brute_force_thread_args *)malloc(num_threads * sizeof(brute_force_thread_args));
    
    if (!threads || !thread_args) {
        if (threads) free(threads);
        if (thread_args) free(thread_args);
        free(boundary_coords);
        free(float_dist_map);
        return NULL;
    }
    
    rows_per_thread = h_out / num_threads;
    
    for (thread_i = 0; thread_i < num_threads; ++thread_i) {
        start_y = thread_i * rows_per_thread;
        end_y = (thread_i == num_threads - 1) ? h_out : (start_y + rows_per_thread);

        thread_args[thread_i].dist_map = float_dist_map;
        thread_args[thread_i].closest_coord = closest_coord;
        thread_args[thread_i].boundary_coords = boundary_coords;
        thread_args[thread_i].img_data = img_data;
        thread_args[thread_i].boundary_count = boundary_count;
        thread_args[thread_i].w_out = w_out;
        thread_args[thread_i].h_out = h_out;
        thread_args[thread_i].w_in = w_in;
        thread_args[thread_i].comp_in = comp;
        thread_args[thread_i].max_coord_val = max_coord_val;
        thread_args[thread_i].start_y = start_y;
        thread_args[thread_i].end_y = end_y;

#ifdef _WIN32
        threads[thread_i] = CreateThread(NULL, 0, generate_sdf_brute_force_thread, &thread_args[thread_i], 0, NULL);
        if (threads[thread_i] == NULL) {
            break;
        }
#else
        if (pthread_create(&threads[thread_i], NULL, generate_sdf_brute_force_thread, &thread_args[thread_i]) != 0) {
            break;
        }
#endif
    }
    
    for (join_i = 0; join_i < thread_i; ++join_i) {
#ifdef _WIN32
        WaitForSingleObject(threads[join_i], INFINITE);
        CloseHandle(threads[join_i]);
#else
        pthread_join(threads[join_i], NULL);
#endif
    }
    
    free(threads);
    free(thread_args);
    free(boundary_coords);

    for (i = 0; i < w_out * h_out; ++i) {
        if (float_dist_map[i] > max_dist) {
            max_dist = float_dist_map[i];
        }
    }

    if (max_dist == 0.0f) {
        max_dist = 1.0f;
    }

    {
        float inv_max_dist = 1.0f / max_dist;
        for (i = 0; i < w_out * h_out; ++i) {
            float_dist_map[i] *= inv_max_dist;
        }
    }
    
    return float_dist_map;
}

static float *generate_sdf_float(const pngl_uc *img_data, int w, int h, int comp) {
    int w_out = w * G_CONFIG.scale_factor;
    int h_out = h * G_CONFIG.scale_factor;
    coord *closest_coord;
    float *float_dist_map = NULL;
    
    closest_coord = (coord *)malloc(w_out * h_out * sizeof(coord));
    if (!closest_coord) {
        return NULL;
    }
    
    if (strcmp(G_CONFIG.sdf_algorithm, "fsm") == 0) {
        float_dist_map = generate_sdf_fsm_2_pass(img_data, w, h, comp, closest_coord);
    } else if (strcmp(G_CONFIG.sdf_algorithm, "mp-fsm") == 0) {
        float_dist_map = generate_sdf_fsm_multi_pass(img_data, w, h, comp, closest_coord);
    } else if (strcmp(G_CONFIG.sdf_algorithm, "jfa") == 0) {
        float_dist_map = generate_sdf_jfa(img_data, w, h, comp, closest_coord);
    } else if (strcmp(G_CONFIG.sdf_algorithm, "brute-force") == 0) {
        float_dist_map = generate_sdf_brute_force(img_data, w, h, comp, closest_coord);
    } else {
        float_dist_map = generate_sdf_fsm_2_pass(img_data, w, h, comp, closest_coord);
    }
    
    free(closest_coord);
    return float_dist_map;
}

static vec3 calculate_normal(const float *height_map, int w, int h, int x, int y) {
    float h_c, h_r, h_l, h_u, h_d;
    float dx, dy;
    vec3 normal;
    float z_scale = G_CONFIG.z_scale;

    h_c = height_map[y * w + x];
    
    h_r = (x < w - 1) ? height_map[y * w + (x + 1)] : h_c;
    h_l = (x > 0)     ? height_map[y * w + (x - 1)] : h_c;
    h_u = (y > 0)     ? height_map[(y - 1) * w + x] : h_c;
    h_d = (y < h - 1) ? height_map[(y + 1) * w + x] : h_c;

    dx = (h_l - h_r) * z_scale;
    dy = (h_u - h_d) * z_scale;
    
    normal.x = -dx;
    normal.y = dy;
    normal.z = 1.0f;
    
    return vec3_normalize(normal);
}

static float calculate_pbr_intensity(vec3 normal, vec3 light_dir_norm, vec3 view_dir, vec3 half_vector_norm, float roughness) {
    float ambient_light = 0.05f; 
    float light_energy = G_CONFIG.pbr_light_intensity;
    float F0 = 0.04f;             
    float PI = M_PI;

    float n_dot_l = fmaxf(0.0f, vec3_dot(normal, light_dir_norm));
    float n_dot_v = fmaxf(0.0f, vec3_dot(normal, view_dir)); 
    float v_dot_h = fmaxf(0.0f, vec3_dot(view_dir, half_vector_norm)); 
    float n_dot_h = fmaxf(0.0f, vec3_dot(normal, half_vector_norm)); 

    if (n_dot_v < 0.0001f || n_dot_l < 0.0001f) {
        return ambient_light;
    }
    if (n_dot_h < 0.0001f) { 
        n_dot_h = 0.0001f;
    }
    
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom_ndf = n_dot_h * n_dot_h * (alpha2 - 1.0f) + 1.0f;
    float d = alpha2 / (PI * denom_ndf * denom_ndf);

    float f_schlick = F0 + (1.0f - F0) * powf(1.0f - v_dot_h, 5.0f);

    float k = alpha / 2.0f;
    float g_v = n_dot_v / (n_dot_v * (1.0f - k) + k);
    float g_l = n_dot_l / (n_dot_l * (1.0f - k) + k);
    float g = g_v * g_l;

    float specular_brdf = d * f_schlick * g / (4.0f * n_dot_v * n_dot_l);
    
    float diffuse_brdf = (1.0f * (1.0f - f_schlick)) / PI; 

    float direct_radiance = (diffuse_brdf + specular_brdf) * n_dot_l * light_energy;
    
    float final_intensity = ambient_light + direct_radiance;
    
    return fmaxf(0.0f, fminf(1.0f, final_intensity)); 
}

static float calculate_gradient_magnitude(const float *height_map, int w, int h, int x, int y) {
    float h_r, h_l, h_u, h_d;
    float dx, dy;

    h_r = (x < w - 1) ? height_map[y * w + (x + 1)] : height_map[y * w + x];
    h_l = (x > 0)     ? height_map[y * w + (x - 1)] : height_map[y * w + x];
    h_u = (y > 0)     ? height_map[(y - 1) * w + x] : height_map[y * w + x];
    h_d = (y < h - 1) ? height_map[(y + 1) * w + x] : height_map[y * w + x];

    dx = (h_r - h_l) / 2.0f;
    dy = (h_d - h_u) / 2.0f;
    
    return sqrt(dx * dx + dy * dy);
}

static int is_local_max(const float *height_map, int w, int h, int x, int y) {
    float center = height_map[y * w + x];
    int dx, dy;

    if (center < 0.001f) return 0; 

    for (dy = -1; dy <= 1; ++dy) {
        for (dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            
            int nx = x + dx;
            int ny = y + dy;
            
            if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                if (height_map[ny * w + nx] >= center) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

static void draw_marker(pngl_uc *output_data, int w, int h, int comp, int center_x, int center_y) {
    int x, y;
    int marker_radius = G_CONFIG.center_marker_size;
    const char *blend_mode = G_CONFIG.point_blending_mode;
    
    int marker_r = (int)(G_CONFIG.center_marker_color[0] * 255.0f);
    int marker_g = (int)(G_CONFIG.center_marker_color[1] * 255.0f);
    int marker_b = (int)(G_CONFIG.center_marker_color[2] * 255.0f);
    
    pngl_uc current_r, current_g, current_b;
    pngl_uc final_r, final_g, final_b;
    
    if (strcmp(blend_mode, "normal") != 0 && comp < 3) {
        blend_mode = "normal";
    }
    
    for (y = center_y - marker_radius; y <= center_y + marker_radius; ++y) {
        for (x = center_x - marker_radius; x <= center_x + marker_radius; ++x) {
            if (x >= 0 && x < w && y >= 0 && y < h) {
                int index = y * w + x;
                int is_marker_pixel = 0;
                
                if (strcmp(G_CONFIG.center_marker_shape, "pixel") == 0) {
                    is_marker_pixel = (x == center_x && y == center_y);
                } else {
                    is_marker_pixel = (x == center_x || y == center_y);
                }
                
                if (is_marker_pixel) {
                    if (comp < 3) {
                        output_data[index] = (pngl_uc)marker_r;
                    } else {
                        int idx = index * comp;
                        current_r = output_data[idx + 0];
                        current_g = output_data[idx + 1];
                        current_b = output_data[idx + 2];
                        
                        final_r = (pngl_uc)marker_r;
                        final_g = (pngl_uc)marker_g;
                        final_b = (pngl_uc)marker_b;

                        if (strcmp(blend_mode, "and") == 0) {
                            final_r = (pngl_uc)(current_r & marker_r);
                            final_g = (pngl_uc)(current_g & marker_g);
                            final_b = (pngl_uc)(current_b & marker_b);
                        } else if (strcmp(blend_mode, "or") == 0) {
                            final_r = (pngl_uc)(current_r | marker_r);
                            final_g = (pngl_uc)(current_g | marker_g);
                            final_b = (pngl_uc)(current_b | marker_b);
                        } else if (strcmp(blend_mode, "xor") == 0) {
                            final_r = (pngl_uc)(current_r ^ marker_r);
                            final_g = (pngl_uc)(current_g ^ marker_g);
                            final_b = (pngl_uc)(current_b ^ marker_b);
                        } else {
                        }

                        output_data[idx + 0] = final_r;
                        output_data[idx + 1] = final_g;
                        output_data[idx + 2] = final_b;
                        
                        if (comp == 4) {
                            output_data[idx + 3] = 255;
                        }
                    }
                }
            }
        }
    }
}

typedef struct {
    int x, y;
} queue_item;

static void write_yaml_output(const char *filename, const shape_data *shape_data, int num_shapes, float scale) {
    FILE *fp;
    int i;

    fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "ERROR: Could not open YAML output file '%s'.\n", filename);
        return;
    }

    fprintf(fp, "metadata:\n");
    fprintf(fp, "  scale_factor: %d\n", G_CONFIG.scale_factor);
    fprintf(fp, "  center_method: %s\n", G_CONFIG.center_method);
    fprintf(fp, "  center_bias_x: %.2f\n", G_CONFIG.center_bias[0]);
    fprintf(fp, "  center_bias_y: %.2f\n", G_CONFIG.center_bias[1]);
    fprintf(fp, "shapes:\n");
    
    for (i = 1; i <= num_shapes; ++i) {
        fprintf(fp, "  - id: %d\n", shape_data[i].label);
        fprintf(fp, "    center_x: %.4f\n", shape_data[i].x / scale);
        fprintf(fp, "    center_y: %.4f\n", shape_data[i].y / scale);
        fprintf(fp, "    max_sdf_dist: %.4f\n", shape_data[i].max_dist);
    }

    fclose(fp);
    printf("YAML data saved to '%s'.\n", filename);
}

static int find_shapes_and_centers(const float *dist_map, int w, int h, shape_data **out_shape_data) {
    int i, x, y;
    int *labels;
    int current_label = 0;
    float boundary_dist_threshold = 0.001f;
    int max_labels = 1024;
    
    float *max_dist;
    center_coord *max_dist_pos;
    float *centroid_sum_x;
    float *centroid_sum_y;
    long *centroid_count;
    shape_data *s_data;
    
    queue_item *queue;
    int head, tail;
    int dx[] = {1, -1, 0, 0};
    int dy[] = {0, 0, 1, -1};
    int n;
    
    labels = (int *)calloc(w * h, sizeof(int));
    max_dist = (float *)calloc(max_labels, sizeof(float));
    max_dist_pos = (center_coord *)calloc(max_labels, sizeof(center_coord));
    centroid_sum_x = (float *)calloc(max_labels, sizeof(float));
    centroid_sum_y = (float *)calloc(max_labels, sizeof(float));
    centroid_count = (long *)calloc(max_labels, sizeof(long));
    s_data = (shape_data *)calloc(max_labels, sizeof(shape_data));

    if (!labels || !max_dist || !max_dist_pos || !centroid_sum_x || !centroid_sum_y || !centroid_count || !s_data) {
        if (labels) free(labels);
        if (max_dist) free(max_dist);
        if (max_dist_pos) free(max_dist_pos);
        if (centroid_sum_x) free(centroid_sum_x);
        if (centroid_sum_y) free(centroid_sum_y);
        if (centroid_count) free(centroid_count);
        if (s_data) free(s_data);
        return 0;
    }

    queue = (queue_item *)malloc(w * h * sizeof(queue_item));
    if (!queue) {
        goto cleanup;
    }
    
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            i = y * w + x;
            
            if (dist_map[i] > boundary_dist_threshold && labels[i] == 0) {
                
                current_label++;
                if (current_label >= max_labels) {
                    current_label = max_labels - 1;
                    goto skip_labeling;
                }
                
                head = 0;
                tail = 0;
                labels[i] = current_label;
                queue[tail].x = x;
                queue[tail].y = y;
                tail++;

                {
                    float current_max_dist = dist_map[i];
                    center_coord current_max_dist_pos;
                    float centroid_acc_x;
                    float centroid_acc_y;
                    long centroid_count_val;
                    
                    current_max_dist_pos.x = (float)x;
                    current_max_dist_pos.y = (float)y;
                    
                    centroid_acc_x = (float)x;
                    centroid_acc_y = (float)y;
                    centroid_count_val = 1;
                    

                    while (head < tail) {
                        queue_item current = queue[head];
                        head++;
                        
                        {
                            float current_pixel_dist = dist_map[current.y * w + current.x];
                            
                            if (current_pixel_dist > current_max_dist) {
                                current_max_dist = current_pixel_dist;
                                current_max_dist_pos.x = (float)current.x;
                                current_max_dist_pos.y = (float)current.y;
                            }
                            
                            if (head != 1) { 
                                centroid_acc_x += (float)current.x;
                                centroid_acc_y += (float)current.y;
                                centroid_count_val++;
                            }
                        }
                        
                        for (n = 0; n < 4; ++n) {
                            int nx = current.x + dx[n];
                            int ny = current.y + dy[n];
                            int ni = ny * w + nx;

                            if (nx >= 0 && nx < w && ny >= 0 && ny < h &&
                                dist_map[ni] > boundary_dist_threshold && labels[ni] == 0) {
                                
                                labels[ni] = current_label;
                                queue[tail].x = nx;
                                queue[tail].y = ny;
                                tail++;
                            }
                        }
                    }
                    
                    max_dist[current_label] = current_max_dist;
                    max_dist_pos[current_label] = current_max_dist_pos;
                    
                    centroid_sum_x[current_label] = centroid_acc_x;
                    centroid_sum_y[current_label] = centroid_acc_y;
                    centroid_count[current_label] = centroid_count_val;
                }
            }
        }
    }

skip_labeling:;
    
    for (i = 1; i <= current_label; ++i) {
        float final_center_x, final_center_y;
        float final_dist_val;
        
        if (centroid_count[i] > 0) {
            
            if (strcmp(G_CONFIG.center_method, "centroid") == 0) {
                final_center_x = centroid_sum_x[i] / (float)centroid_count[i];
                final_center_y = centroid_sum_y[i] / (float)centroid_count[i];
                
                {
                    int log_x = (int)(final_center_x + 0.5f);
                    int log_y = (int)(final_center_y + 0.5f);
                    if (log_x >= w || log_x < 0) log_x = w-1;
                    if (log_y >= h || log_y < 0) log_y = h-1;
                    final_dist_val = dist_map[log_y * w + log_x];
                }

                printf("Found centroid for Shape %d at (%.1f, %.1f) with SDF dist %.2f\n",
                       i, final_center_x, final_center_y, final_dist_val);
            } else {
                final_center_x = max_dist_pos[i].x;
                final_center_y = max_dist_pos[i].y;
                final_dist_val = max_dist[i];
                
                printf("Found max-dist center for Shape %d at (%.1f, %.1f) with max dist %.2f\n",
                       i, final_center_x, final_center_y, final_dist_val);
            }
            
            final_center_x += G_CONFIG.center_bias[0];
            final_center_y += G_CONFIG.center_bias[1];
            
            s_data[i].label = i;
            s_data[i].x = final_center_x;
            s_data[i].y = final_center_y;
            s_data[i].max_dist = final_dist_val;
        }
    }

    *out_shape_data = s_data;

cleanup:
    free(labels);
    free(max_dist);
    free(max_dist_pos);
    free(centroid_sum_x);
    free(centroid_sum_y);
    free(centroid_count);
    if (queue) free(queue);
    
    return current_label;
}

static pngl_uc *create_empty_canvas(int w_out, int h_out, int *output_comp, int transparent_mode) {
    int i, total_pixels = w_out * h_out;
    pngl_uc *canvas;
    pngl_uc bg_r = (pngl_uc)(G_CONFIG.bg_color[0] * 255.0f);
    pngl_uc bg_g = (pngl_uc)(G_CONFIG.bg_color[1] * 255.0f);
    pngl_uc bg_b = (pngl_uc)(G_CONFIG.bg_color[2] * 255.0f);
    pngl_uc bg_a = 255;

    *output_comp = 4;
    
    canvas = (pngl_uc *)malloc(total_pixels * (*output_comp) * sizeof(pngl_uc));
    if (!canvas) {
        return NULL;
    }

    if (transparent_mode != TRANSPARENT_MODE_OFF) {
        bg_a = 0;
    }

    for (i = 0; i < total_pixels; ++i) {
        int idx = i * (*output_comp);
        canvas[idx + 0] = bg_r;
        canvas[idx + 1] = bg_g;
        canvas[idx + 2] = bg_b;
        canvas[idx + 3] = bg_a;
    }
    return canvas;
}

static pngl_uc *generate_base_image_data(const float *float_dist_map, int w_out, int h_out, unsigned int base_mode, int *output_comp, const pngl_uc *original_data, int orig_comp, const vec3 *normals) {
    int i, total_pixels = w_out * h_out;
    pngl_uc *base_data;
    float inv_gamma = 1.0f / G_CONFIG.gamma;
    vec3 light_dir_norm = {0.0f, 0.0f, 0.0f};
    vec3 view_dir = {0.0f, 0.0f, 1.0f}; 
    vec3 half_vec_norm = {0.0f, 0.0f, 0.0f};
    
    int alpha_required = (G_CONFIG.base_transparent_mode != TRANSPARENT_MODE_OFF) || (base_mode == RENDER_MODE_ORIGINAL && orig_comp == 4);
    
    if (base_mode == RENDER_MODE_ORIGINAL && !alpha_required) {
        *output_comp = 3;
    } else {
        *output_comp = 4;
        alpha_required = 1;
    }
    
    base_data = (pngl_uc *)malloc(total_pixels * (*output_comp) * sizeof(pngl_uc));
    if (!base_data) {
        return NULL;
    }

    if (base_mode == RENDER_MODE_SOLID_BG) {
        pngl_uc bg_r = (pngl_uc)(G_CONFIG.bg_color[0] * 255.0f);
        pngl_uc bg_g = (pngl_uc)(G_CONFIG.bg_color[1] * 255.0f);
        pngl_uc bg_b = (pngl_uc)(G_CONFIG.bg_color[2] * 255.0f);
        pngl_uc bg_a = 255;
        
        if (G_CONFIG.base_transparent_mode != TRANSPARENT_MODE_OFF) {
             bg_a = 0;
        }

        for (i = 0; i < total_pixels; ++i) {
            int idx = i * (*output_comp);
            base_data[idx + 0] = bg_r;
            base_data[idx + 1] = bg_g;
            base_data[idx + 2] = bg_b;
            if (*output_comp == 4) {
                base_data[idx + 3] = bg_a;
            }
        }
        return base_data;
    }


    if (base_mode & RENDER_MODE_LIT) {
        light_dir_norm = vec3_normalize(G_CONFIG.light_dir);
        half_vec_norm.x = view_dir.x + light_dir_norm.x;
        half_vec_norm.y = view_dir.y + light_dir_norm.y;
        half_vec_norm.z = view_dir.z + light_dir_norm.z;
        half_vec_norm = vec3_normalize(half_vec_norm);
    }

    for (i = 0; i < total_pixels; ++i) {
        float normalized_dist = float_dist_map[i];
        float boosted_dist = normalized_dist * G_CONFIG.quality_boost;
        float final_dist_clamped = boosted_dist > 1.0f ? 1.0f : boosted_dist;
        float final_intensity = 1.0f;
        int x = i % w_out;
        int y = i / w_out;
        int idx = i * (*output_comp);
        pngl_uc final_r = 0, final_g = 0, final_b = 0;
        pngl_uc alpha = 255;
        vec3 normal = {0.0f, 0.0f, 1.0f};

        if (normals) {
            normal = normals[i];
        }

        if (G_CONFIG.gamma != 1.0f) {
            final_dist_clamped = pow(final_dist_clamped, inv_gamma);
        }
        
        if (base_mode & RENDER_MODE_LIT) {
            final_intensity = calculate_pbr_intensity(normal, light_dir_norm, view_dir, half_vec_norm, G_CONFIG.roughness);
            
             
            {
                float t = final_dist_clamped;
                float r, g, b;

                
                r = G_CONFIG.color_start[0] * (1.0f - t) + G_CONFIG.color_end[0] * t;
                g = G_CONFIG.color_start[1] * (1.0f - t) + G_CONFIG.color_end[1] * t;
                b = G_CONFIG.color_start[2] * (1.0f - t) + G_CONFIG.color_end[2] * t;

                 
                final_r = (pngl_uc)(r * final_intensity * 255.0f);
                final_g = (pngl_uc)(g * final_intensity * 255.0f);
                final_b = (pngl_uc)(b * final_intensity * 255.0f);

                
                if (normalized_dist < 0.001f && (G_CONFIG.bg_color[0] != 0.0f || G_CONFIG.bg_color[1] != 0.0f || G_CONFIG.bg_color[2] != 0.0f)) {
                    final_r = (pngl_uc)(G_CONFIG.bg_color[0] * 255.0f);
                    final_g = (pngl_uc)(G_CONFIG.bg_color[1] * 255.0f);
                    final_b = (pngl_uc)(G_CONFIG.bg_color[2] * 255.0f);
                }
            }
        } else if (base_mode & RENDER_MODE_NORMAL) {
            final_r = (pngl_uc)((normal.x * 0.5f + 0.5f) * 255.0f);
            final_g = (pngl_uc)((normal.y * 0.5f + 0.5f) * 255.0f);
            final_b = (pngl_uc)((normal.z * 0.5f + 0.5f) * 255.0f);
        } else if (base_mode & RENDER_MODE_MASK) {
            int bx = x / G_CONFIG.scale_factor;
            int by = y / G_CONFIG.scale_factor;
            int mask_val = get_pixel_luminance(original_data, bx, by, w_out / G_CONFIG.scale_factor, orig_comp);
            final_r = final_g = final_b = (pngl_uc)mask_val;
        } else if (base_mode & RENDER_MODE_MAX_DIST) {
            final_r = final_g = final_b = (pngl_uc)(255.0f * normalized_dist); 
        } else if (base_mode & RENDER_MODE_LOCAL_MAX) {
            int is_max = is_local_max(float_dist_map, w_out, h_out, x, y);
            final_r = is_max ? 255 : 0;
            final_g = is_max ? 0 : 0;
            final_b = is_max ? 0 : 0;
        } else if (base_mode & RENDER_MODE_GRADIENT) {
            float grad_mag = calculate_gradient_magnitude(float_dist_map, w_out, h_out, x, y);
            pngl_uc val = (pngl_uc)fminf(255.0f, grad_mag * 255.0f * 5.0f); 
            final_r = final_g = final_b = 255 - val; 
        } else if (base_mode & RENDER_MODE_ORIGINAL) {
            int bx = x / G_CONFIG.scale_factor;
            int by = y / G_CONFIG.scale_factor;
            int orig_idx = (by * (w_out / G_CONFIG.scale_factor) + bx) * orig_comp;
            
            final_r = original_data[orig_idx + 0];
            final_g = original_data[orig_idx + 1];
            final_b = original_data[orig_idx + 2];
            
            if (orig_comp == 4 && alpha_required) {
                alpha = original_data[orig_idx + 3];
            }
        } else if (base_mode & RENDER_MODE_SDF) {
            float t = final_dist_clamped;
            float r, g, b;

            r = G_CONFIG.color_start[0] * (1.0f - t) + G_CONFIG.color_end[0] * t;
            g = G_CONFIG.color_start[1] * (1.0f - t) + G_CONFIG.color_end[1] * t;
            b = G_CONFIG.color_start[2] * (1.0f - t) + G_CONFIG.color_end[2] * t;
            
            final_r = (pngl_uc)(r * final_intensity * 255.0f);
            final_g = (pngl_uc)(g * final_intensity * 255.0f);
            final_b = (pngl_uc)(b * final_intensity * 255.0f);

            if (normalized_dist < 0.001f && (G_CONFIG.bg_color[0] != 0.0f || G_CONFIG.bg_color[1] != 0.0f || G_CONFIG.bg_color[2] != 0.0f)) {
                final_r = (pngl_uc)(G_CONFIG.bg_color[0] * 255.0f);
                final_g = (pngl_uc)(G_CONFIG.bg_color[1] * 255.0f);
                final_b = (pngl_uc)(G_CONFIG.bg_color[2] * 255.0f);
            }
        }

        
        base_data[idx + 0] = final_r;
        base_data[idx + 1] = final_g;
        base_data[idx + 2] = final_b;
        
        
        if (alpha_required) {
            
            if (base_mode & RENDER_MODE_ORIGINAL) {
                 
            } else {
                
                 
                if (G_CONFIG.base_transparent_mode == TRANSPARENT_MODE_PURE_BLACK) {
                    if (normalized_dist < 0.001f) {
                        alpha = 0; 
                    } else {
                        alpha = 255;
                    }
                } else if (G_CONFIG.base_transparent_mode == TRANSPARENT_MODE_SDF_FALLOFF) {
                    
                    float linear_alpha_float = normalized_dist;
                    if (linear_alpha_float > 1.0f) linear_alpha_float = 1.0f;
                    
                    alpha = (pngl_uc)(linear_alpha_float * 255.0f);
                }
            }

            base_data[idx + 3] = alpha;
        }
    }

    return base_data;
}

static pngl_uc *generate_final_image(const float *float_dist_map, int w_out, int h_out, unsigned int render_flags, const pngl_uc *original_data, int orig_comp, const vec3 *normals, const shape_data *s_data, int num_shapes) {
    unsigned int base_mode = render_flags & 0x0FFF;
    unsigned int point_flags = render_flags & 0xF000;
    int base_comp;
    pngl_uc *base_data = NULL;
    int i;
    pngl_uc *final_data;

    if (base_mode == RENDER_MODE_NONE && point_flags & RENDER_OVERLAY_POINTS) {
         
        base_data = create_empty_canvas(w_out, h_out, &base_comp, G_CONFIG.base_transparent_mode);
    } else {
         
        base_data = generate_base_image_data(float_dist_map, w_out, h_out, base_mode, &base_comp, original_data, orig_comp, normals);
    }
    
    if (!base_data) {
        return NULL;
    }

    final_data = base_data;
    
    if (point_flags & RENDER_OVERLAY_POINTS) {
    	for (i = 1; i <= num_shapes; ++i) {
      	draw_marker(final_data, w_out, h_out, base_comp, (int)(s_data[i].x + 0.5f), (int)(s_data[i].y + 0.5f));
      }
    }

    return final_data;
}

static const char *get_mode_name(unsigned int flag) {
    switch (flag) {
        case RENDER_MODE_ORIGINAL: return "original";
        case RENDER_MODE_SDF: return "sdf";
        case RENDER_MODE_NORMAL: return "normal";
        case RENDER_MODE_LIT: return "lit";
        case RENDER_MODE_MASK: return "mask";
        case RENDER_MODE_MAX_DIST: return "max-dist";
        case RENDER_MODE_LOCAL_MAX: return "local-max";
        case RENDER_MODE_GRADIENT: return "gradient";
        case RENDER_MODE_SOLID_BG: return "solid-bg";
        case RENDER_MODE_NONE: return "empty";
        default: return "unknown";
    }
}

static int write_image_from_flags(unsigned int final_render_flags, int w_out, int h_out, const pngl_uc *original_data, int orig_comp, const float *float_dist_map, const vec3 *normals, const shape_data *s_data, int num_shapes) {
    char output_path[MAX_PATH_CUSTOM];
    char mode_name[256];
    char *base_name;
    char *dot;
    pngl_uc *output_data;
    int success;
    
    unsigned int base_mode = final_render_flags & 0x0FFF;
    unsigned int is_overlay = final_render_flags & RENDER_OVERLAY_POINTS;

    strcpy(mode_name, get_mode_name(base_mode));
    
    if (is_overlay) {
        if (base_mode != RENDER_MODE_NONE) {
            strcat(mode_name, "_plus_points");
        } else {
             
             if (G_CONFIG.base_transparent_mode != TRANSPARENT_MODE_OFF) {
                 strcpy(mode_name, "points_alpha");
             } else {
                 strcpy(mode_name, "points_solid");
             }
        }
    } else {
        
    }

    strcpy(output_path, G_CONFIG.output_filename);
    base_name = strrchr(output_path, PATH_SEP);
    base_name = base_name ? (base_name + 1) : output_path;
    
    dot = strrchr(base_name, '.');
    if (dot) *dot = '\0';

    strcat(base_name, "_");
    strcat(base_name, mode_name);
    strcat(base_name, ".png");

    output_data = generate_final_image(float_dist_map, w_out, h_out, final_render_flags, original_data, orig_comp, normals, s_data, num_shapes);
	
    if (output_data == NULL) {
        return 0;
    }
    
    {
        int final_comp = 3;
        
         
        if (base_mode == RENDER_MODE_ORIGINAL && orig_comp == 3) {
             final_comp = 3;
        } else if (base_mode == RENDER_MODE_NONE && G_CONFIG.base_transparent_mode == TRANSPARENT_MODE_OFF) {
             final_comp = 3; 
        } else {
             final_comp = 4;
        }


        printf("Generated Output (%s): %dx%d (Comp: %d) -> '%s'\n", mode_name, w_out, h_out, final_comp, output_path);
        success = pngl_write_png(output_path, w_out, h_out, final_comp, output_data, 0);
    }
    
    free(output_data);
    return success;
}

int main(int argc, char **argv) {
    int w, h, comp;
    int w_out, h_out;
    pngl_uc *img_data = NULL;
    pngl_uc *original_img_data_copy = NULL;
    float *float_dist_map = NULL;
    vec3 *normals = NULL;
    FILE *fp;
    struct ArgParseState state;
    int arg_error;
    long len;
    shape_data *s_data = NULL;
    int num_shapes = 0;
    
    arg_error = argparse_process_args(&state, argv, argc, G_OPTIONS);

    if (arg_error) {
        return 1;
    }
    
    if (!platform_init()) {
        return 1;
    }

    fp = fopen(G_CONFIG.input_filename, "rb");
    if (!fp) {
        fprintf(stderr, "ERROR: Could not open input file '%s'.\n", G_CONFIG.input_filename);
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    img_data = (pngl_uc *)malloc(len);
    if (!img_data) {
        fclose(fp);
        return 1;
    }

    if (fread(img_data, 1, len, fp) != len) {
        fclose(fp);
        free(img_data);
        return 1;
    }
    fclose(fp);

    {
        int dummy_comp;
        pngl_uc *loaded_data = pngl_load_from_memory(img_data, (int)len, &w, &h, &dummy_comp, 4);
        free(img_data);
        img_data = loaded_data;
        comp = 4;
    }

    if (img_data == NULL) {
        fprintf(stderr, "ERROR loading image: %s\n", pngl_failure_reason());
        return 1;
    }
    
    if (G_CONFIG.render_flags & RENDER_MODE_ORIGINAL || G_CONFIG.render_flags & RENDER_MODE_MASK) {
        original_img_data_copy = (pngl_uc *)malloc(w * h * comp);
        if (!original_img_data_copy) {
            pngl_image_free(img_data);
            return 1;
        }
        memcpy(original_img_data_copy, img_data, w * h * comp);
    }

    printf("Input: %s (%dx%d, Comp: %d)\n", G_CONFIG.input_filename, w, h, comp);
    printf("Scale: %d, Threshold: %.2f, Quality: %.2f\n", G_CONFIG.scale_factor, G_CONFIG.black_threshold, G_CONFIG.quality_boost);

    preprocess_image(img_data, w, h, comp);

    float_dist_map = generate_sdf_float(img_data, w, h, comp);
    
    w_out = w * G_CONFIG.scale_factor;
    h_out = h * G_CONFIG.scale_factor;

    if (float_dist_map == NULL) {
        pngl_image_free(img_data);
        if (original_img_data_copy) free(original_img_data_copy);
        return 1;
    }

    
    if (G_CONFIG.render_flags & RENDER_MODE_LIT || G_CONFIG.render_flags & RENDER_MODE_NORMAL || G_CONFIG.render_flags & RENDER_MODE_GRADIENT) {
        int i;
        normals = (vec3 *)malloc(w_out * h_out * sizeof(vec3));
        if (normals) {
            printf("Generating surface normals...\n");
            for (i = 0; i < w_out * h_out; ++i) {
                normals[i] = calculate_normal(float_dist_map, w_out, h_out, i % w_out, i / w_out);
            }
        }
    }
    
    
    if (G_CONFIG.show_center_points || G_CONFIG.yaml_output_filename) {
         
         num_shapes = find_shapes_and_centers(float_dist_map, w_out, h_out, &s_data);
    }

    if (G_CONFIG.yaml_output_filename && num_shapes > 0) {
        write_yaml_output(G_CONFIG.yaml_output_filename, s_data, num_shapes, (float)G_CONFIG.scale_factor);
    }
    
    {
        unsigned int base_mode = G_CONFIG.render_flags & 0x0FFF;
        
        if (base_mode) {
             
            unsigned int base_only_flags = base_mode;
            if (write_image_from_flags(base_only_flags, w_out, h_out, original_img_data_copy, comp, float_dist_map, normals, s_data, num_shapes) == 0) {
            }
             
            if (G_CONFIG.show_center_points) {
                
                unsigned int base_plus_points_flags = base_mode | RENDER_OVERLAY_POINTS;
                if (write_image_from_flags(base_plus_points_flags, w_out, h_out, original_img_data_copy, comp, float_dist_map, normals, s_data, num_shapes) == 0) {
                }
            }
        } else if (G_CONFIG.show_center_points) {
             
             unsigned int points_only_flags = RENDER_MODE_NONE | RENDER_OVERLAY_POINTS;
             if (write_image_from_flags(points_only_flags, w_out, h_out, original_img_data_copy, comp, float_dist_map, normals, s_data, num_shapes) == 0) {
             }
        }
        
        if (base_mode == RENDER_MODE_NONE && !G_CONFIG.show_center_points) {
             fprintf(stderr, "WARNING: No output mode selected. Use --base-output-mode or --center-points.\n");
        }
    }

    
    pngl_image_free(img_data);
    free(float_dist_map);
    if (original_img_data_copy) free(original_img_data_copy);
    if (normals) free(normals);
    if (s_data) free(s_data);

    return 0;
}