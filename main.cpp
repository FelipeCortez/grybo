#include <iostream>
#include <string>
#include <sstream>
#include <GL/gl3w.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <math.h>
#include <random>
#include "audio.h"
#include "midi.h"
#include "shader.h"
#include "shapes.h"
#include "model.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl_gl3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const unsigned int SCREEN_WIDTH  = 800;
const unsigned int SCREEN_HEIGHT = 600;
const float SIDES_INCREMENT = 0.005f;
const float UPDOWN_INCREMENT = 0.005f;
const unsigned int NOTES = 5;
const float noteHitTreshold = 0.05f;

float currentBPM = 120.0f;
float seconds_offset = 0.0f;

double rangeMap(double input, double inputStart, double inputEnd, double outputStart, double outputEnd) {
  double slope = (outputEnd - outputStart) / (inputEnd - inputStart);
  return outputStart + (slope * (input - inputStart));
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Song name please" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::string songId(argv[1]);
  std::stringstream fullPath;
  fullPath << "songs/" << songId << "/" << songId << ".mid";
  std::cout << fullPath.str() << std::endl;

  float strumBarOffset = 1.350f;
  auto gameSong = getSongFromMidiFile(fullPath.str());
  std::cout << "first note: " << gameSong.gameNotes[0].zPosition << std::endl;

  Audio* audio = new Audio(songId);
  audio->audioData->startDelay = gameSong.startDelay;

  SDL_Window* window = NULL;

  if (SDL_Init(SDL_INIT_VIDEO  |
               SDL_INIT_EVENTS |
               SDL_INIT_JOYSTICK |
               SDL_INIT_TIMER) != 0) {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    return 1;
  }

  window = SDL_CreateWindow("Grybo",
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            SCREEN_WIDTH,
                            SCREEN_HEIGHT,
                            SDL_WINDOW_OPENGL);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

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

  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  ImGui_ImplSdlGL3_Init(window);

  // Setup style
  ImGui::StyleColorsDark();

  PlaneShape strumBarPlaneShape = initPlane(false);
  PlaneShape planeShape = initPlane();

  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                          (float) SCREEN_WIDTH / (float) SCREEN_HEIGHT,
                                          0.1f,
                                          100.0f);

  glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  Model noteModel("assets/note.obj");

  int width, height, nrChannels;
  unsigned int measureThickTexture, measureThinTexture, noteHitTexture;

  glGenTextures(1, &measureThickTexture);
  glBindTexture(GL_TEXTURE_2D, measureThickTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
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

  // --

  glGenTextures(1, &measureThinTexture);
  glBindTexture(GL_TEXTURE_2D, measureThinTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  data = stbi_load("assets/measure-thin.png", &width, &height, &nrChannels, 0);

  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "Failed to load texture" << std::endl;
  }

  stbi_image_free(data);

  // --

  glGenTextures(1, &noteHitTexture);
  glBindTexture(GL_TEXTURE_2D, noteHitTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  data = stbi_load("assets/note-hit.png", &width, &height, &nrChannels, 0);

  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "Failed to load texture" << std::endl;
  }

  stbi_image_free(data);

  // --

  Shader strumBarShader("assets/strum.vert", "assets/strum.frag");
  Shader fretboardShader("assets/fretboard.vert", "assets/fretboard.frag");
  fretboardShader.use();
  fretboardShader.setInt("texture2d", 0);

  Shader modelShader("assets/model.vert", "assets/model.frag");

  SDL_Event e;

  uint32_t time, pastTime, startTime;
  float dt;

  startTime = 0;
  time = SDL_GetTicks();

  bool quit;
  quit = false;

  const Uint8* keys = NULL;
  keys = SDL_GetKeyboardState(NULL);

  SDL_GameController *ctrl;
  SDL_Joystick *joy;
  int i;

  for(i = 0; i < SDL_NumJoysticks(); ++i) {
    if (SDL_IsGameController(i)) {
      printf("Index \'%i\' is a compatible controller, named \'%s\'\n", i, SDL_GameControllerNameForIndex(i));
      ctrl = SDL_GameControllerOpen(i);
      joy = SDL_GameControllerGetJoystick(ctrl);
    } else {
      printf("Index \'%i\' is not a compatible controller.\n", i);
    }
  }

  // game stuff
  float upDownValue = 0.0f;
  float sidesValue = 0.0f;
  float strumZ = 0.0f;

  int firstNote = 0;
  int currentNote = 0;

  float notePositions[NOTES] = {0};

  for (i = 0; i < NOTES; ++i) {
    notePositions[i] = rangeMap(i, 0.0f, NOTES - 1, -0.39f, 0.39f);
  }

  while(!quit) {
    pastTime = time;
    time = SDL_GetTicks();
    dt = (time - pastTime) / 1000.0f;

    while (SDL_PollEvent(&e)) {
      ImGui_ImplSdlGL3_ProcessEvent(&e);

      if (e.type == SDL_JOYAXISMOTION) {
        // std::cout << "hmm" << std::endl;
      } else if (e.type == SDL_JOYBUTTONDOWN) {
        std::cout << "yay" << std::endl;
      } else if (e.type == SDL_KEYDOWN) {
        switch(e.key.keysym.sym) {
        case SDLK_UP:
          upDownValue += UPDOWN_INCREMENT;
          break;
        case SDLK_DOWN:
          upDownValue -= UPDOWN_INCREMENT;
          break;
        case SDLK_LEFT:
          sidesValue -= SIDES_INCREMENT;
          break;
        case SDLK_RIGHT:
          sidesValue += SIDES_INCREMENT;
          break;
        default:
          break;
        }
      } else if (e.type == SDL_QUIT) {
        quit = true;
      }
    }

    static float fogZ = 3.5f;
    static float fogBand = 4.0f;

    ImGui_ImplSdlGL3_NewFrame(window);

    ImGui::SliderFloat("fogZ", &fogZ, -5.0f, 5.0f, "%.3f");
    ImGui::SliderFloat("fogBand", &fogBand, 0.1f, 5.0f, "%.3f");

    strumZ = msToPos(time, gameSong);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, measureThickTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, measureThinTexture);

    view = glm::mat4();
    view = glm::rotate(view, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::translate(view, glm::vec3(0.0f, -0.7f, strumZ - strumBarOffset));

    fretboardShader.use();

    fretboardShader.setMat4("view",  view);
    fretboardShader.setMat4("projection", projection);
    fretboardShader.setFloat("fogZ", fogZ);
    fretboardShader.setFloat("fogBand", fogBand);

    for (i = 0; i < 1000; ++i) {
      drawQuarter(fretboardShader, planeShape, i, (i % 4) == 0);
    }

    modelShader.use();
    modelShader.setMat4("view",  view);
    modelShader.setMat4("projection", projection);
    modelShader.setFloat("fogZ", fogZ);
    modelShader.setFloat("fogBand", fogBand);

    const float scaleFactor = 0.08f + upDownValue;

    // discards past notes
    while (firstNote < gameSong.gameNotes.size()) {
      auto& note = gameSong.gameNotes[firstNote];
      if (note.zPosition > strumZ - strumBarOffset) { break; }
      ++firstNote;
    }

    // draws from first note to last in sight
    currentNote = firstNote;
    while (currentNote < gameSong.gameNotes.size()) {
      auto& note = gameSong.gameNotes[currentNote];

      if (note.zPosition > strumZ + fogZ + fogBand) { break; }

      glm::mat4 model(1.0f);
      model = glm::translate(model, glm::vec3(notePositions[note.note],
                                              0.0f,
                                              -note.zPosition));
      model = glm::scale(model, glm::vec3(scaleFactor));
      modelShader.setMat4("model", model);

      // G: 0x74b544
      // R: 0xdd3e3e
      // Y: 0xddd43e
      // B: 0x3e85dd
      // O: 0xdd983e
      glm::vec4 noteColor;
      switch (note.note) {
      case 0:
        noteColor = glm::vec4(0x74 / 255.0f, 0xb5 / 255.0f, 0x44 / 255.0f, 1.0f);
        break;
      case 1:
        noteColor = glm::vec4(0xdd / 255.0f, 0x3e / 255.0f, 0x3e / 255.0f, 1.0f);
        break;
      case 2:
        noteColor = glm::vec4(0xdd / 255.0f, 0xd4 / 255.0f, 0x3e / 255.0f, 1.0f);
        break;
      case 3:
        noteColor = glm::vec4(0x3e / 255.0f, 0x85 / 255.0f, 0xdd / 255.0f, 1.0f);
        break;
      case 4:
        noteColor = glm::vec4(0xdd / 255.0f, 0x98 / 255.0f, 0x3e / 255.0f, 1.0f);
        break;
      default:
        noteColor = glm::vec4(0xdd / 255.0f, 0x98 / 255.0f, 0x3e / 255.0f, 1.0f);
      }


      modelShader.setVec4("noteColor", noteColor);
      noteModel.Draw(modelShader);

      ++currentNote;
    }

    strumBarShader.use();
    strumBarShader.setMat4("view",  view);
    strumBarShader.setMat4("projection", projection);
    drawStrumBar(strumBarShader, strumBarPlaneShape, strumZ);

    for (auto note : gameSong.gameNotes) {
      if (fabs(note.zPosition - strumZ) < noteHitTreshold) {
        // cout << note.note << endl;
        drawNoteHit(strumBarShader, strumBarPlaneShape, strumZ, note.note);
      }
    }

    // ImGui::SliderFloat("song pos", &strumZ, -1.0f, 30.0f, "%.3f");
    ImGui::SliderFloat("strum bar", &strumBarOffset, -5.0f, 5.0f, "%.3f");
    ImGui::Text("strumZ = %f", strumZ);
    // ImGui::Text("audio  = %f", (float) audio->audioData->pos / 44100);
    ImGui::Text("time   = %d", time);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);

    ImGui::Render();
    ImGui_ImplSdlGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);
  }

  ImGui_ImplSdlGL3_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(glContext);
  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}
