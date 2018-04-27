#ifndef SHAPES_H
#define SHAPES_H

#include <GL/gl3w.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

float boxVertices[] = {
  -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
   0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
   0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
   0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
  -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
   0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
   0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
   0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
  -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

  -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
  -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
  -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

   0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
   0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
   0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
   0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
   0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
   0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

  -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
   0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
   0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
   0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
  -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

  -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
   0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
   0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
   0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
  -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
  -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

struct BoxShape {
  unsigned int VAOBox;
  unsigned int VBOBox;
};

BoxShape initBox() {
  BoxShape boxShape;

  glGenVertexArrays(1, &boxShape.VAOBox);
  glBindVertexArray(boxShape.VAOBox);

  glGenBuffers(1, &boxShape.VBOBox);
  glBindBuffer(GL_ARRAY_BUFFER, boxShape.VBOBox);
  glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  return boxShape;
}

void drawBox(Shader ourShader, BoxShape& boxShape, glm::vec3 pos, float scale) {
  glm::mat4 model(1.0f);
  model = glm::scale(model, glm::vec3(scale));
  model = glm::translate(model, pos);
  ourShader.setMat4("model", model);

  glBindVertexArray(boxShape.VAOBox);
  glBindBuffer(GL_ARRAY_BUFFER, boxShape.VBOBox);
  glDrawArrays(GL_TRIANGLES, 0, 36);
}

float planeVertices[] = {
  0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
  -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
  -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
};

unsigned int indices[] = {
  0, 1, 3,
  1, 2, 3
};

struct PlaneShape {
  unsigned int VAOPlane;
  unsigned int VBOPlane;
  unsigned int EBOPlane;
  bool textured;
};

PlaneShape initPlane(bool textured = true) {
  PlaneShape planeShape;

  glGenVertexArrays(1, &planeShape.VAOPlane);
  glBindVertexArray(planeShape.VAOPlane);

  glGenBuffers(1, &planeShape.VBOPlane);
  glGenBuffers(1, &planeShape.EBOPlane);

  glBindBuffer(GL_ARRAY_BUFFER, planeShape.VBOPlane);
  glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeShape.EBOPlane);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  if (textured) {
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }

  planeShape.textured = textured;
  return planeShape;
}

void drawStrumBar(Shader ourShader, PlaneShape planeShape, float pos) {
  glm::mat4 model(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 0.0f, -pos));
  model = glm::translate(model, glm::vec3(0.0f, 0.001f, 0.0f));
  model = glm::scale(model, glm::vec3(1.0f, 1.0f, 0.05f)); // shrink in z
  model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

  ourShader.setMat4("model", model);

  glBindVertexArray(planeShape.VAOPlane);
  glBindBuffer(GL_ARRAY_BUFFER, planeShape.VBOPlane);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void drawNoteHit(Shader ourShader, PlaneShape planeShape, float pos, int note) {
  glm::mat4 model(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 0.0f, -pos));
  model = glm::scale(model, glm::vec3(1.0f / 5.0f));
  model = glm::translate(model, glm::vec3((note - 2), 0.0f, 0.0f));
  model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f));

  ourShader.setMat4("model", model);

  glBindVertexArray(planeShape.VAOPlane);
  glBindBuffer(GL_ARRAY_BUFFER, planeShape.VBOPlane);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void drawQuarter(Shader ourShader, PlaneShape planeShape, float pos, bool thick) {
  glm::mat4 model(1.0f);

  model = glm::translate(model, glm::vec3(0.0f, 0.0f, -pos));
  model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.5f)); // origin at bottom
  model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

  if (thick) {
    ourShader.setInt("texture2d", 0);
  } else {
    ourShader.setInt("texture2d", 1);
  }

  ourShader.setMat4("model", model);

  glBindVertexArray(planeShape.VAOPlane);
  glBindBuffer(GL_ARRAY_BUFFER, planeShape.VBOPlane);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

#endif
