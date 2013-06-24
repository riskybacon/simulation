//
//  SettingsIcon.cpp
//  Singularity
//
//  Created by Jeffrey Bowles on 12/26/12.
//  Copyright (c) 2012 Jeffrey Bowles. All rights reserved.
//

#include "SettingsIcon.h"
#include <vector>
#include <glm/glm.hpp>

using glm::vec4;
using glm::vec2;
using std::vector;

SettingsIcon::SettingsIcon(GLuint posAttrib, GLuint tcAttrib)
{
   vector<vec4> pos =
   {
      vec4(-1, -1, 0, 1),
      vec4(-1,  1, 0, 1),
      vec4( 1, -1, 0, 1),
      vec4( 1,  1, 0, 1)
   };
   
   size_t posSize = sizeof(vec4) * pos.size();
   
   vector<vec2> tc =
   {
      vec2( 0, 0),
      vec2( 0, 1),
      vec2( 1, 0),
      vec2( 1, 1)
   };
   
   size_t tcSize = sizeof(vec2) * tc.size();
   
   glGenVertexArrays(1, &_vao);
   glBindVertexArray(_vao);
   glGenBuffers(1, &_buffer);
   glGenBuffers(1, &_tcBuffer);

   glBindBuffer(GL_ARRAY_BUFFER, _buffer);
   glBufferData(GL_ARRAY_BUFFER, posSize, &pos[0], GL_STATIC_DRAW);
   glVertexAttribPointer(posAttrib, 4, GL_FLOAT, GL_FALSE, 0, NULL);
   glEnableVertexAttribArray(posAttrib);
   
   glBindBuffer(GL_ARRAY_BUFFER, _tcBuffer);
   glBufferData(GL_ARRAY_BUFFER, tcSize, &tc[0], GL_STATIC_DRAW);
   glVertexAttribPointer(tcAttrib, 2, GL_FLOAT, GL_FALSE, 0, NULL);
   glEnableVertexAttribArray(tcAttrib);
#if 0
   glBindBuffer(GL_ARRAY_BUFFER, _buffer);
   
   
   glBufferData(GL_ARRAY_BUFFER, posSize + tcSize, NULL, GL_STATIC_DRAW);
   
   glBufferSubData(GL_ARRAY_BUFFER, 0,       posSize, &pos[0]);
   glBufferSubData(GL_ARRAY_BUFFER, posSize, tcSize,  &tc[0]);
   
   glVertexAttribPointer(posAttrib, 4, GL_FLOAT, GL_FALSE, 0, NULL);
   glVertexAttribPointer(tcAttrib,  2, GL_FLOAT, GL_FALSE, 0, (void*)posSize);
#endif
   
}

void SettingsIcon::draw()
{
   glBindVertexArray(_vao);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}