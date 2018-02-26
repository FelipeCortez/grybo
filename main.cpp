#include <iostream>
#include <GL/gl3w.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <math.h>
#include <random>
#include "shader.h"
#include "shapes.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

double rangeMap(double input, double inputStart, double inputEnd, double outputStart, double outputEnd) {
  double slope = (outputEnd - outputStart) / (inputEnd - inputStart);
  return outputStart + (slope * (input - inputStart));
}

const unsigned int SCREEN_WIDTH  = 800;
const unsigned int SCREEN_HEIGHT = 600;
const float SCROLL_SPEED = 3.0f;
const unsigned int NOTES = 5;

std::default_random_engine generator;
std::uniform_int_distribution<int> noteDistribution(0, NOTES);
std::uniform_int_distribution<int> posDistribution(0, 1000);
auto getRandomNote = std::bind(noteDistribution, generator);
auto getRandomPos = std::bind(posDistribution, generator);

typedef struct Note {
  int which;
  float posZ;
} Note;

int main(int argc, char* args[]) {
  SDL_Window* window = NULL;

  if (SDL_Init(SDL_INIT_VIDEO  |
               SDL_INIT_AUDIO  |
               SDL_INIT_EVENTS |
               SDL_INIT_TIMER) != 0) {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    return 1;
  }

  window = SDL_CreateWindow("Hero",
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            SCREEN_WIDTH,
                            SCREEN_HEIGHT,
                            SDL_WINDOW_OPENGL);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
  SDL_GLContext glContext = SDL_GL_CreateContext(window);
  SDL_GL_SetSwapInterval(1);
  SDL_GL_MakeCurrent(window, glContext);

  if (gl3wInit()) {
    fprintf(stderr, "failed to initialize OpenGL\n");
    return 1;
  }

  if (!gl3wIsSupported(3, 4)) {
    fprintf(stderr, "OpenGL 3.4 not supported\n");
    return 1;
  }

  BoxShape boxShape = initBox();
  PlaneShape planeShape = initPlane();

  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                          (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
                                          0.1f,
                                          100.0f);

  glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  glEnable(GL_DEPTH_TEST);

  Shader ourShader("assets/1.vert", "assets/1.frag");

  int width, height, nrChannels;
  unsigned int measureThickTexture, measureThinTexture;

  glGenTextures(1, &measureThickTexture);
  glBindTexture(GL_TEXTURE_2D, measureThickTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_set_flip_vertically_on_load(true);
  unsigned char* data = stbi_load("assets/measure-thick.png", &width, &height, &nrChannels, 0);

  if (data) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
  } else {
      std::cout << "Failed to load texture" << std::endl;
  }

  stbi_image_free(data);

  glGenTextures(1, &measureThinTexture);
  glBindTexture(GL_TEXTURE_2D, measureThinTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  data = stbi_load("assets/measure-thin.png", &width, &height, &nrChannels, 0);

  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "Failed to load texture" << std::endl;
  }

  stbi_image_free(data);

  ourShader.use();
  ourShader.setInt("texture2d", 0);

  SDL_Event e;

  Uint32 time, pastTime;
  float dt;

  time = SDL_GetTicks();

  bool quit;
  quit = false;

  const Uint8* keys = NULL;
  keys = SDL_GetKeyboardState(NULL);

  // game stuff

  float upDownValue = -0.7;
  float sidesValue = 15;
  float cameraZ = -5.0f;

  int i;

  Note randomNotes[1000];
  float notePositions[NOTES] = {0};

  for (i = 0; i < NOTES; ++i) {
    notePositions[i] = rangeMap(i, 0.0f, NOTES - 1, -4.0f, 4.0f);
  }

  for (i = 0; i < 1000; ++i) {
    randomNotes[i] = { getRandomNote(), (float)getRandomPos() * 5.0f };
  }

  while(!quit) {
    pastTime = time;
    time = SDL_GetTicks();
    dt = (time - pastTime) / 1000.0f;

    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_KEYDOWN) {
        switch(e.key.keysym.sym) {
        case SDLK_UP:
          upDownValue += 0.1f;
          break;
        case SDLK_DOWN:
          upDownValue -= 0.1f;
          break;
        case SDLK_LEFT:
          sidesValue -= 0.5f;
          break;
        case SDLK_RIGHT:
          sidesValue += 0.5f;
          break;
        default:
          break;
        }
      } else if (e.type == SDL_QUIT) {
        quit = true;
      }
    }

    cameraZ += SCROLL_SPEED * dt;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, measureThickTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, measureThinTexture);

    // activate shader
    ourShader.use();

    //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::mat4(1.0f);
    view = glm::rotate(view, glm::radians(sidesValue), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::translate(view, glm::vec3(0.0f, upDownValue, cameraZ));

    ourShader.setMat4("view",  view);
    ourShader.setMat4("projection", projection);

    for (i = 0; i < 1000; ++i) {
      drawPlane(ourShader, planeShape, i, (i % 4) == 0);
    }

    for (i = 0; i < 1000; ++i) {
      drawBox(ourShader, boxShape, glm::vec3(notePositions[randomNotes[i].which], 0.5f, -randomNotes[i].posZ), 0.10);
    }

    std::cout << sidesValue << " | " << upDownValue << std::endl;
    //std::cout << dt << std::endl;

    SDL_GL_SwapWindow(window);
  }

  /*
  glDeleteVertexArrays(1, &VAOPlane);
  glDeleteBuffers(1, &VBOPlane);
  glDeleteBuffers(1, &EBOPlane);

  glDeleteVertexArrays(1, &VAOBox);
  glDeleteBuffers(1, &VBOBox);
  */

  SDL_GL_DeleteContext(glContext);
  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}
