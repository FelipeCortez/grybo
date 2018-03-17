#include <iostream>
#include <GL/gl3w.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <math.h>
#include <random>
#include <soundio/soundio.h>
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

#define AUDIO_FORMAT AUDIO_S16LSB
#define AUDIO_RATE 48000
#define AUDIO_CHANNELS 2
#define AUDIO_BLOCK 256
#define TWO_PI (3.14159265f * 2.0f)

const unsigned int SCREEN_WIDTH  = 800;
const unsigned int SCREEN_HEIGHT = 600;
const float SIDES_INCREMENT = 0.005f;
const float UPDOWN_INCREMENT = 0.005f;
const float STRUM_BAR_POSITION = 0.6f;
const unsigned int NOTES = 5;

float currentBPM = 120.0f;
float seconds_offset = 0.0f;

double rangeMap(double input, double inputStart, double inputEnd, double outputStart, double outputEnd) {
  double slope = (outputEnd - outputStart) / (inputEnd - inputStart);
  return outputStart + (slope * (input - inputStart));
}

void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max) {
    const struct SoundIoChannelLayout *layout = &outstream->layout;
    float float_sample_rate = outstream->sample_rate;
    float seconds_per_frame = 1.0f / float_sample_rate;
    struct SoundIoChannelArea *areas;
    int frames_left = frame_count_max;
    int err;

    while (frames_left > 0) {
        int frame_count = frames_left;

        if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count))) {
            fprintf(stderr, "%s\n", soundio_strerror(err));
            exit(1);
        }

        if (!frame_count)
            break;

        float pitch = 440.0f;
        float radians_per_second = pitch * TWO_PI;
        for (int frame = 0; frame < frame_count; frame += 1) {
            float sample = sinf((seconds_offset + frame * seconds_per_frame) * radians_per_second);
            for (int channel = 0; channel < layout->channel_count; channel += 1) {
                float *ptr = (float*)(areas[channel].ptr + areas[channel].step * frame);
                *ptr = sample;
            }
        }
        seconds_offset = fmodf(seconds_offset +
            seconds_per_frame * frame_count, 1.0f);

        if ((err = soundio_outstream_end_write(outstream))) {
            fprintf(stderr, "%s\n", soundio_strerror(err));
            exit(1);
        }

        frames_left -= frame_count;
    }
}

int main(int argc, char* args[]) {
  int err;
  struct SoundIo *soundio = soundio_create();
  if (!soundio) {
    fprintf(stderr, "out of memory\n");
    return 1;
  }

  if ((err = soundio_connect(soundio))) {
    fprintf(stderr, "error connecting: %s", soundio_strerror(err));
    return 1;
  }

  soundio_flush_events(soundio);

  int default_out_device_index = soundio_default_output_device_index(soundio);
  if (default_out_device_index < 0) {
    fprintf(stderr, "no output device found");
    return 1;
  }

  struct SoundIoDevice *device = soundio_get_output_device(soundio, default_out_device_index);
  if (!device) {
    fprintf(stderr, "out of memory");
    return 1;
  }

  fprintf(stderr, "Output device: %s\n", device->name);

  struct SoundIoOutStream *outstream = soundio_outstream_create(device);
  outstream->format = SoundIoFormatFloat32NE;
  outstream->write_callback = write_callback;

  if ((err = soundio_outstream_open(outstream))) {
    fprintf(stderr, "unable to open device: %s", soundio_strerror(err));
    return 1;
  }

  if (outstream->layout_error)
    fprintf(stderr, "unable to set channel layout: %s\n", soundio_strerror(outstream->layout_error));

  if ((err = soundio_outstream_start(outstream))) {
    fprintf(stderr, "unable to start device: %s", soundio_strerror(err));
    return 1;
  }

  // ---------------------------------------

  SDL_Window* window = NULL;

  if (SDL_Init(SDL_INIT_VIDEO  |
               SDL_INIT_EVENTS |
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
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
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
                                          (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
                                          0.1f,
                                          100.0f);

  glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  glEnable(GL_DEPTH_TEST);

  Model noteModel("assets/note.obj");

  int width, height, nrChannels;
  unsigned int measureThickTexture, measureThinTexture;

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

  Shader strumBarShader("assets/strum.vert", "assets/strum.frag");
  Shader fretboardShader("assets/fretboard.vert", "assets/fretboard.frag");
  fretboardShader.use();
  fretboardShader.setInt("texture2d", 0);

  Shader modelShader("assets/model.vert", "assets/model.frag");

  SDL_Event e;

  uint32_t time, pastTime;
  float dt;

  time = SDL_GetTicks();

  bool quit;
  quit = false;

  const Uint8* keys = NULL;
  keys = SDL_GetKeyboardState(NULL);

  // game stuff

  float upDownValue = 0.0f;
  float sidesValue = 0.0f;
  float cameraZ = -3.0f;

  int i;

  float notePositions[NOTES] = {0};

  for (i = 0; i < NOTES; ++i) {
    notePositions[i] = rangeMap(i, 0.0f, NOTES - 1, -0.39f, 0.39f);
  }

  auto gameSong = getSongFromMidiFile("assets/ovo.mid");

  while(!quit) {
    pastTime = time;
    time = SDL_GetTicks();
    dt = (time - pastTime) / 1000.0f;

    while (SDL_PollEvent(&e)) {
      ImGui_ImplSdlGL3_ProcessEvent(&e);

      if (e.type == SDL_KEYDOWN) {
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

    static float fogZ = 1.2f;

    ImGui_ImplSdlGL3_NewFrame(window);

    ImGui::SliderFloat("fogZ", &fogZ, -5.0f, 5.0f, "ratio = %.3f");

    cameraZ = msToPos(time, gameSong);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, measureThickTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, measureThinTexture);

    view = glm::mat4();
    view = glm::rotate(view, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::translate(view, glm::vec3(0.0f, -0.7f, cameraZ));

    fretboardShader.use();

    fretboardShader.setMat4("view",  view);
    fretboardShader.setMat4("projection", projection);
    fretboardShader.setFloat("fogZ", fogZ);

    for (i = 0; i < 1000; ++i) {
      drawPlane(fretboardShader, planeShape, i, (i % 4) == 0);
    }

    modelShader.use();
    modelShader.setMat4("view",  view);
    modelShader.setMat4("projection", projection);
    modelShader.setFloat("fogZ", fogZ);

    const float scaleFactor = 0.08f + upDownValue;

    for (auto note : gameSong.gameNotes) {
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
    }

    strumBarShader.use();
    strumBarShader.setMat4("view",  view);
    strumBarShader.setMat4("projection", projection);
    drawPlane(strumBarShader, strumBarPlaneShape, cameraZ + STRUM_BAR_POSITION, false);

    ImGui::Text("cameraZ = %f", cameraZ);
    ImGui::Text("time    = %d", time);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);

    ImGui::Render();
    ImGui_ImplSdlGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);
  }

  /*
  glDeleteVertexArrays(1, &VAOPlane);
  glDeleteBuffers(1, &VBOPlane);
  glDeleteBuffers(1, &EBOPlane);

  glDeleteVertexArrays(1, &VAOBox);
  glDeleteBuffers(1, &VBOBox);
  */
  ImGui_ImplSdlGL3_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(glContext);
  SDL_DestroyWindow(window);

  SDL_Quit();

  soundio_outstream_destroy(outstream);
  soundio_device_unref(device);
  soundio_destroy(soundio);

  return 0;
}
