/**
* Author: Connor Chavez
* Assignment: Pong Clone
* Date due: 2025-3-01, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>

enum AppStatus { RUNNING, TERMINATED };

constexpr float WINDOW_SIZE_MULT = 1.2f;

constexpr int WINDOW_WIDTH = 640 * WINDOW_SIZE_MULT,
WINDOW_HEIGHT = 480 * WINDOW_SIZE_MULT;

constexpr float BG_RED = 0.9765625f,
BG_GREEN = 0.97265625f,
BG_BLUE = 0.9609375f,
BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr GLint NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL = 0;
constexpr GLint TEXTURE_BORDER = 0;

constexpr float MILLISECONDS_IN_SECOND = 1000.0;
// Source:https://cyberpunk.fandom.com/wiki/Mantis_Blades
constexpr char LEFT_PADDLE_SPRITE_FILEPATH[] = "MantisBladesLeft.png",
RIGHT_PADDLE_SPRITE_FILEPATH[] = "MantisBladesRight.png",
// Source: https://www.reddit.com/r/LowSodiumCyberpunk/comments/krbs72/made_a_skippy_desktop_icon_for_my_game_lol_love_it/
BALL_SPRITE_FILEPATH[] = "Skippy.png",
// Source:https://pnghunter.com/png/cyberpunk-2077-logo/
LOGO_SPRITE_FILEPATH[] = "Cyberpunk2077Logo.png",
// Source:https://www.fontbolt.com/font/cyberpunk-font/
PLAYER1_WIN_SPRITE_FILEPATH[] = "Player1.png",
PLAYER2_WIN_SPRITE_FILEPATH[] = "Player2.png",
// Source: https://wallpapers.com/background/cyberpunk-2077-background-be8xyzvaav5ifwg5.html
BACKGROUND_SPRITE_FILEPATH[] = "Cyberpunk2077BG.jpg";

//constexpr float MINIMUM_COLLISION_DISTANCE = 1.0f;

constexpr glm::vec3 INIT_SCALE_BALL = glm::vec3(1.0f, 1.0f, 0.0f),
INIT_POS_BALL = glm::vec3(0.0f, 0.0f, 0.0f),
INIT_SCALE_PADDLE = glm::vec3(1.0f, 2.5f, 0.0f),
INIT_POS_LEFT_PADDLE = glm::vec3(-4.0f, 0.0f, 0.0f),
INIT_POS_RIGHT_PADDLE = glm::vec3(4.0f, 0.0f, 0.0f),
INIT_POS_LOGO = glm::vec3(0.0f, 3.2f, 0.0f),
INIT_SCALE_LOGO = glm::vec3(4.5f, 1.0f, 0.0f),
INIT_POS_WIN = glm::vec3(0.0f, 0.0f, 0.0f),
INIT_SCALE_WIN = glm::vec3(4.5f, 1.5f, 0.0f),
INIT_POS_BG = glm::vec3(0.0f, 0.0f, 0.0f),
INIT_SCALE_BG = glm::vec3(10.0f, 9.0f, 0.0f);

SDL_Window* g_display_window;

AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();
glm::mat4 g_view_matrix, 
g_left_paddle_matrix, 
g_right_paddle_matrix, 
g_projection_matrix, 
g_ball_matrix,
g_ball2_matrix,
g_ball3_matrix,
g_logo_matrix, 
g_win_matrix,
g_bg_matrix;

float g_previous_ticks = 0.0f;

GLuint g_left_paddle_texture_id,
g_right_paddle_texture_id,
g_ball_texture_id,
g_logo_texture_id,
g_win1_texture_id,
g_win2_texture_id,
g_bg_texture_id;

constexpr float BALL_SPEED = 1.5f;
constexpr float PLAYER_SPEED = 2.5f;


glm::vec3 g_left_paddle_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_left_paddle_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_right_paddle_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_right_paddle_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_ball_movement = glm::vec3(-1.5f, 0.0f, 0.0f);
glm::vec3 g_ball2_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_ball2_movement = glm::vec3(-1.5f, 0.3f, 0.0f);
glm::vec3 g_ball3_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_ball3_movement = glm::vec3(-1.5f, -0.3f, 0.0f);

void initialise();
void process_input();
void update();
void render();
void shutdown();

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Cyberpong 2077",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);


    if (g_display_window == nullptr) shutdown();

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_left_paddle_matrix = glm::mat4(1.0f);
    g_right_paddle_matrix = glm::mat4(1.0f);
    g_ball_matrix = glm::mat4(1.0f);
    g_ball2_matrix = glm::mat4(1.0f);
    g_ball3_matrix = glm::mat4(1.0f);

    g_logo_matrix = glm::mat4(1.0f);
    g_logo_matrix = glm::translate(g_logo_matrix, INIT_POS_LOGO);
    g_logo_matrix = glm::scale(g_logo_matrix, INIT_SCALE_LOGO);

    g_win_matrix = glm::mat4(1.0f);
    g_win_matrix = glm::translate(g_win_matrix, INIT_POS_WIN);
    g_win_matrix = glm::scale(g_win_matrix, INIT_SCALE_WIN);

    g_bg_matrix = glm::mat4(1.0f);
    g_bg_matrix = glm::translate(g_bg_matrix, INIT_POS_BG);
    g_bg_matrix = glm::scale(g_bg_matrix, INIT_SCALE_BG);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    g_left_paddle_texture_id = load_texture(LEFT_PADDLE_SPRITE_FILEPATH);
    g_right_paddle_texture_id = load_texture(RIGHT_PADDLE_SPRITE_FILEPATH);
    g_ball_texture_id = load_texture(BALL_SPRITE_FILEPATH);
    g_logo_texture_id = load_texture(LOGO_SPRITE_FILEPATH);
    g_win1_texture_id = load_texture(PLAYER1_WIN_SPRITE_FILEPATH);
    g_win2_texture_id = load_texture(PLAYER2_WIN_SPRITE_FILEPATH);
    g_bg_texture_id = load_texture(BACKGROUND_SPRITE_FILEPATH);


    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

bool single_player = false;
int num_balls = 1;

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_app_status = TERMINATED;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_q: g_app_status = TERMINATED; break;
            default: break;
            }

        default:
            break;
        }
    }


    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    
    if (key_state[SDL_SCANCODE_T] && !single_player){
        single_player=true;
        g_right_paddle_movement.y = 1.0f;
    }

    if (key_state[SDL_SCANCODE_1]) {
        num_balls = 1;
    }else if (key_state[SDL_SCANCODE_2]) {
        num_balls = 2;
    }else if (key_state[SDL_SCANCODE_3]) {
        num_balls = 3;
    }

    
    
    if (key_state[SDL_SCANCODE_W] && g_left_paddle_position.y + INIT_SCALE_PADDLE.y/2.0f <4.0f) {
        g_left_paddle_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_S]&& g_left_paddle_position.y - INIT_SCALE_PADDLE.y / 2.0f >-4.0f) {
        g_left_paddle_movement.y = -1.0f;
    }

    if (!single_player) {
        if (key_state[SDL_SCANCODE_UP] && g_right_paddle_position.y + INIT_SCALE_PADDLE.y / 2.0f < 4.0f) {
            g_right_paddle_movement.y = 1.0f;
        }
        else if (key_state[SDL_SCANCODE_DOWN] && g_right_paddle_position.y - INIT_SCALE_PADDLE.y / 2.0f > -4.0f) {
            g_right_paddle_movement.y = -1.0f;
        }
    }
    
    


    
}

constexpr float min_collision_distance = 1.0f;
bool can_ball1_collide = true,
can_ball2_collide = true,
can_ball3_collide = true;


float BALL_BOTTOM,
PADDLE_TOP,
PADDLE_BOTTOM,
left_x_distance,
left_y_distance,
right_x_distance,
right_y_distance;

bool player_1_win = false,
player_2_win = false;
    

void update()
{
    // --- DELTA TIME CALCULATIONS --- //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    // --- ACCUMULATOR LOGIC --- //
    if (!player_1_win && !player_2_win) {
        g_ball_position += g_ball_movement * BALL_SPEED * delta_time;
        if (num_balls >= 2) {
            g_ball2_position += g_ball2_movement * BALL_SPEED * delta_time;
        }
        if (num_balls == 3) {
            g_ball3_position += g_ball3_movement * BALL_SPEED * delta_time;
        }

        g_left_paddle_position += g_left_paddle_movement * PLAYER_SPEED * delta_time;
        g_right_paddle_position += g_right_paddle_movement * PLAYER_SPEED * delta_time;
    }


    

    // --- TRANSLATION --- //
    g_left_paddle_matrix = glm::mat4(1.0f);
    g_left_paddle_matrix = glm::translate(g_left_paddle_matrix, INIT_POS_LEFT_PADDLE);
    g_left_paddle_matrix = glm::translate(g_left_paddle_matrix, g_left_paddle_position);


    g_right_paddle_matrix = glm::mat4(1.0f);
    g_right_paddle_matrix = glm::translate(g_right_paddle_matrix, INIT_POS_RIGHT_PADDLE);
    g_right_paddle_matrix = glm::translate(g_right_paddle_matrix, g_right_paddle_position);

    g_left_paddle_movement = glm::vec3(0.0f);

    if (!single_player) {
        g_right_paddle_movement = glm::vec3(0.0f);
    }
    else if (g_right_paddle_position.y + INIT_SCALE_PADDLE.y / 2.0f > 4.0f) {
        g_right_paddle_movement.y = -1.0f;
        //g_right_paddle_position.y = 4.0f;
    }
    else if (g_right_paddle_position.y - INIT_SCALE_PADDLE.y / 2.0f < -4.0f) {
        g_right_paddle_movement.y = 1.0f;
        //g_right_paddle_position.y = -4.0f;
    }
    

    g_ball_matrix = glm::mat4(1.0f);
    g_ball_matrix = glm::translate(g_ball_matrix, INIT_POS_BALL);
    g_ball_matrix = glm::translate(g_ball_matrix, g_ball_position);

    g_ball2_matrix = glm::mat4(1.0f);
    g_ball2_matrix = glm::translate(g_ball2_matrix, INIT_POS_BALL);
    g_ball2_matrix = glm::translate(g_ball2_matrix, g_ball2_position);

    g_ball3_matrix = glm::mat4(1.0f);
    g_ball3_matrix = glm::translate(g_ball3_matrix, INIT_POS_BALL);
    g_ball3_matrix = glm::translate(g_ball3_matrix, g_ball3_position);
    
   
    

    // --- SCALING --- //
    g_left_paddle_matrix = glm::scale(g_left_paddle_matrix, INIT_SCALE_PADDLE);
    g_right_paddle_matrix = glm::scale(g_right_paddle_matrix, INIT_SCALE_PADDLE);

    g_ball_matrix = glm::scale(g_ball_matrix, INIT_SCALE_BALL);
    g_ball2_matrix = glm::scale(g_ball2_matrix, INIT_SCALE_BALL);
    g_ball3_matrix = glm::scale(g_ball3_matrix, INIT_SCALE_BALL);

    // --- COLLISION LOGIC --- //

    //Ball1
    

    left_x_distance = fabs(g_ball_position.x + INIT_POS_BALL.x - INIT_POS_LEFT_PADDLE.x - 
        g_left_paddle_position.x)- ((INIT_SCALE_BALL.x + INIT_SCALE_PADDLE.x) / 2.0f);
    left_y_distance = fabs(g_ball_position.y + INIT_POS_BALL.y - INIT_POS_LEFT_PADDLE.y - 
        g_left_paddle_position.y) - ((INIT_SCALE_BALL.y + INIT_SCALE_PADDLE.y) / 2.0f);

    right_x_distance = fabs(g_ball_position.x + INIT_POS_BALL.x - INIT_POS_RIGHT_PADDLE.x
        - g_right_paddle_position.x) - ((INIT_SCALE_BALL.x + INIT_SCALE_PADDLE.x) / 2.0f);
    right_y_distance = fabs(g_ball_position.y + INIT_POS_BALL.y - INIT_POS_RIGHT_PADDLE.y - 
        g_right_paddle_position.y) - ((INIT_SCALE_BALL.y + INIT_SCALE_PADDLE.y) / 2.0f);

    if (left_x_distance > 0 && right_x_distance > 0) {
        can_ball1_collide = true;
    }

    

    if (left_x_distance < 0 && left_y_distance < 0 && can_ball1_collide) {
        can_ball1_collide = false;
        g_ball_movement.x = -g_ball_movement.x + 0.1f;
        //g_ball_position.x = INIT_POS_LEFT_PADDLE.x + INIT_SCALE_PADDLE.x;

        BALL_BOTTOM = g_ball_position.y;
        PADDLE_BOTTOM = g_left_paddle_position.y - INIT_SCALE_PADDLE.y * (1.0f / 6.0f);
        PADDLE_TOP = g_left_paddle_position.y + INIT_SCALE_PADDLE.y * (1.0f / 6.0f);

        if (BALL_BOTTOM < PADDLE_BOTTOM) {
            g_ball_movement.y = -0.8f;

        }
        else if (BALL_BOTTOM > PADDLE_TOP) {
            g_ball_movement.y = 0.8f;
        }

    }

    if (right_x_distance < 0 && right_y_distance < 0 && can_ball1_collide) {
        can_ball1_collide = false;
        g_ball_movement.x = -g_ball_movement.x - 0.1f;
        //g_ball_position.x = INIT_POS_RIGHT_PADDLE.x - INIT_SCALE_PADDLE.x;

        BALL_BOTTOM = g_ball_position.y;
        PADDLE_BOTTOM = g_right_paddle_position.y - INIT_SCALE_PADDLE.y * (1.0f / 6.0f);
        PADDLE_TOP = g_right_paddle_position.y + INIT_SCALE_PADDLE.y * (1.0f / 6.0f);

        if (BALL_BOTTOM < PADDLE_BOTTOM) {
            g_ball_movement.y = -0.7f;
        }
        else if (BALL_BOTTOM > PADDLE_TOP) {
            g_ball_movement.y = 0.7f;
        }

    }

        

    
    


    if (g_ball_position.y > 3.0f) {
        g_ball_movement.y *= -1.0f;
        g_ball_position.y = 3.0f;
    }
    else if (g_ball_position.y < -3.0f) {
        g_ball_movement.y *= -1.0f;
        g_ball_position.y = -3.0f;
    }
    
    if (g_ball_position.x > 4.0f) {
        g_ball_movement.x = -1.0f;
    }

    //Ball2
    left_x_distance = fabs(g_ball2_position.x + INIT_POS_BALL.x - INIT_POS_LEFT_PADDLE.x - g_left_paddle_position.x)
        - ((INIT_SCALE_BALL.x + INIT_SCALE_PADDLE.x) / 2.0f);
    left_y_distance = fabs(g_ball2_position.y + INIT_POS_BALL.y - INIT_POS_LEFT_PADDLE.y - g_left_paddle_position.y)
        - ((INIT_SCALE_BALL.y + INIT_SCALE_PADDLE.y) / 2.0f);

    right_x_distance = fabs(g_ball2_position.x + INIT_POS_BALL.x - INIT_POS_RIGHT_PADDLE.x - g_right_paddle_position.x)
        - ((INIT_SCALE_BALL.x + INIT_SCALE_PADDLE.x) / 2.0f);
    right_y_distance = fabs(g_ball2_position.y + INIT_POS_BALL.y - INIT_POS_RIGHT_PADDLE.y - g_right_paddle_position.y)
        - ((INIT_SCALE_BALL.y + INIT_SCALE_PADDLE.y) / 2.0f);

    if (left_x_distance > 0 && right_x_distance > 0) {
        can_ball2_collide = true;
    }

    
    if (left_x_distance < 0 && left_y_distance < 0 && can_ball2_collide) {
        can_ball2_collide = false;
        g_ball2_movement.x = -g_ball2_movement.x + 0.1f;
        g_ball2_position.x = INIT_POS_LEFT_PADDLE.x + INIT_SCALE_PADDLE.x;

        BALL_BOTTOM = g_ball2_position.y;
        PADDLE_BOTTOM = g_left_paddle_position.y - INIT_SCALE_PADDLE.y * (1.0f / 6.0f);
        PADDLE_TOP = g_left_paddle_position.y + INIT_SCALE_PADDLE.y * (1.0f / 6.0f);

        if (BALL_BOTTOM < PADDLE_BOTTOM) {
            g_ball2_movement.y = -0.8f;

        }
        else if (BALL_BOTTOM > PADDLE_TOP) {
            g_ball2_movement.y = 0.8f;
        }

    }

    if (right_x_distance < 0 && right_y_distance < 0 && can_ball2_collide) {
        can_ball2_collide = false;
        g_ball2_movement.x = -g_ball2_movement.x - 0.1f;
        g_ball2_position.x = INIT_POS_RIGHT_PADDLE.x - INIT_SCALE_PADDLE.x;

        BALL_BOTTOM = g_ball2_position.y;
        PADDLE_BOTTOM = g_right_paddle_position.y - INIT_SCALE_PADDLE.y * (1.0f / 6.0f);
        PADDLE_TOP = g_right_paddle_position.y + INIT_SCALE_PADDLE.y * (1.0f / 6.0f);

        if (BALL_BOTTOM < PADDLE_BOTTOM) {
            g_ball2_movement.y = -0.7f;
        }
        else if (BALL_BOTTOM > PADDLE_TOP) {
            g_ball2_movement.y = 0.7f;
        }

    }


    if (g_ball2_position.y > 3.0f) {
        g_ball2_movement.y *= -1.0f;
        g_ball2_position.y = 3.0f;
    }
    else if (g_ball2_position.y < -3.0f) {
        g_ball2_movement.y *= -1.0f;
        g_ball2_position.y = -3.0f;
    }

    if (g_ball2_position.x > 4.0f) {
        g_ball2_movement.x = -1.0f;
    }

    //Ball3
    left_x_distance = fabs(g_ball3_position.x + INIT_POS_BALL.x - INIT_POS_LEFT_PADDLE.x - g_left_paddle_position.x)
        - ((INIT_SCALE_BALL.x + INIT_SCALE_PADDLE.x) / 2.0f);
    left_y_distance = fabs(g_ball3_position.y + INIT_POS_BALL.y - INIT_POS_LEFT_PADDLE.y - g_left_paddle_position.y)
        - ((INIT_SCALE_BALL.y + INIT_SCALE_PADDLE.y) / 2.0f);

    right_x_distance = fabs(g_ball3_position.x + INIT_POS_BALL.x - INIT_POS_RIGHT_PADDLE.x - g_right_paddle_position.x)
        - ((INIT_SCALE_BALL.x + INIT_SCALE_PADDLE.x) / 2.0f);
    right_y_distance = fabs(g_ball3_position.y + INIT_POS_BALL.y - INIT_POS_RIGHT_PADDLE.y - g_right_paddle_position.y)
        - ((INIT_SCALE_BALL.y + INIT_SCALE_PADDLE.y) / 2.0f);

    if (left_x_distance > 0 && right_x_distance > 0) {
        can_ball3_collide = true;
    }
    
    if (left_x_distance < 0 && left_y_distance < 0 && can_ball3_collide) {
        can_ball3_collide = false;
        g_ball3_movement.x = -g_ball3_movement.x + 0.1f;
        g_ball3_position.x = INIT_POS_LEFT_PADDLE.x + INIT_SCALE_PADDLE.x;

        BALL_BOTTOM = g_ball3_position.y;
        PADDLE_BOTTOM = g_left_paddle_position.y - INIT_SCALE_PADDLE.y * (1.0f / 6.0f);
        PADDLE_TOP = g_left_paddle_position.y + INIT_SCALE_PADDLE.y * (1.0f / 6.0f);

        if (BALL_BOTTOM < PADDLE_BOTTOM) {
            g_ball3_movement.y = -0.8f;

        }
        else if (BALL_BOTTOM > PADDLE_TOP) {
            g_ball3_movement.y = 0.8f;
        }

    }

    if (right_x_distance < 0 && right_y_distance < 0 && can_ball3_collide) {
        can_ball3_collide = false;
        g_ball3_movement.x = -g_ball3_movement.x - 0.1f;
        g_ball3_position.x = INIT_POS_RIGHT_PADDLE.x - INIT_SCALE_PADDLE.x;

        BALL_BOTTOM = g_ball3_position.y;
        PADDLE_BOTTOM = g_right_paddle_position.y - INIT_SCALE_PADDLE.y * (1.0f / 6.0f);
        PADDLE_TOP = g_right_paddle_position.y + INIT_SCALE_PADDLE.y * (1.0f / 6.0f);

        if (BALL_BOTTOM < PADDLE_BOTTOM) {
            g_ball3_movement.y = -0.7f;
        }
        else if (BALL_BOTTOM > PADDLE_TOP) {
            g_ball3_movement.y = 0.7f;
        }

    }


    if (g_ball3_position.y > 3.0f) {
        g_ball3_movement.y *= -1.0f;
        g_ball3_position.y = 3.0f;
    }
    else if (g_ball3_position.y < -3.0f) {
        g_ball3_movement.y *= -1.0f;
        g_ball3_position.y = -3.0f;
    }

    if (g_ball3_position.x > 4.0f) {
        g_ball3_movement.x = -1.0f;
    }
    

    // --- TERMINATE GAME --- //
   
    if (g_ball_position.x < INIT_POS_LEFT_PADDLE.x) {
        player_2_win = true;
    }
    else if (g_ball_position.x > INIT_POS_RIGHT_PADDLE.x) {
        player_1_win = true;
    }

    if (g_ball2_position.x < INIT_POS_LEFT_PADDLE.x) {
        player_2_win = true;
    }
    else if (g_ball2_position.x > INIT_POS_RIGHT_PADDLE.x) {
        player_1_win = true;
    }
    
    if (g_ball3_position.x < INIT_POS_LEFT_PADDLE.x) {
        player_2_win = true;
    }
    else if (g_ball3_position.x > INIT_POS_RIGHT_PADDLE.x) {
        player_1_win = true;
    }
    
    
}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    // Bind texture
    draw_object(g_bg_matrix, g_bg_texture_id);
    draw_object(g_logo_matrix, g_logo_texture_id);
    draw_object(g_ball_matrix, g_ball_texture_id);
    draw_object(g_left_paddle_matrix, g_left_paddle_texture_id);
    draw_object(g_right_paddle_matrix, g_right_paddle_texture_id);
    
    if (num_balls >= 2) {
        draw_object(g_ball2_matrix, g_ball_texture_id);
    }

    if (num_balls >= 3) {
        draw_object(g_ball3_matrix, g_ball_texture_id);
    }
    
    if (player_1_win) {
        draw_object(g_win_matrix, g_win1_texture_id);
    }
    else if (player_2_win) {
        draw_object(g_win_matrix, g_win2_texture_id);
    }

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
