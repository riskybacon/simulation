//
//  ParticleSystemView.cpp
//  Singularity
//
//  Created by Jeffrey Bowles on 1/16/13.
//  Copyright (c) 2013 Jeffrey Bowles. All rights reserved.
//

#include "ParticleSystemView.h"
#include "ParticleSystemModel.h"

using glm::vec4;

/*
 * Constructor
 *
 * Initialize vertex array objects, vertex buffer objects,
 * clear color and depth clear value
 */
ParticleSystemView::ParticleSystemView(GLuint posAttr, std::shared_ptr<ParticleSystemModel> model)
:  _posAttr       (posAttr)
,  _model         (model)
,  _pVAO(0)
,  _pBO(0)
{
   glGenVertexArrays(1, &_pVAO);
   glBindVertexArray(_pVAO);
   glGenBuffers(1, &_pBO);
   glBindBuffer(GL_ARRAY_BUFFER, _pBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * _model->getNumVertices(), _model->getData(), GL_DYNAMIC_DRAW);
   glVertexAttribPointer(_posAttr, 4, GL_FLOAT, GL_FALSE, 0, NULL);
   glEnableVertexAttribArray(_posAttr);
   glBindVertexArray(0);
}

/*
 * Destructor
 */
ParticleSystemView::~ParticleSystemView()
{
   glDeleteBuffers(1, &_pBO);
   glDeleteVertexArraysOES(1, &_pVAO);
}

/**
 * Update particle positions
 */
void ParticleSystemView::update()
{
   glBindBuffer(GL_ARRAY_BUFFER, _pBO);
   glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4) * _model->getNumVertices(), _model->getData());
   glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/*
 * Draw the particle system
 */
void ParticleSystemView::draw()
{
   glBindVertexArray(_pVAO);
   glDrawArrays(GL_POINTS, 0, _model->getNumVertices());
   GL_ERR_CHECK();
   
}

