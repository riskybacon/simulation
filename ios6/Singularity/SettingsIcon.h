//
//  SettingsIcon.h
//  Singularity
//
//  Created by Jeffrey Bowles on 12/26/12.
//  Copyright (c) 2012 Jeffrey Bowles. All rights reserved.
//

#ifndef __Singularity__SettingsIcon__
#define __Singularity__SettingsIcon__

#include "OpenGL.h"

class SettingsIcon
{
public:
   SettingsIcon(GLuint posAttrib, GLuint tcAttrib);
   void draw();

private:
   GLuint   _vao;
   GLuint   _buffer;
   GLuint   _tcBuffer;
};
#endif /* defined(__Singularity__SettingsIcon__) */
