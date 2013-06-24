//
//  Shader.fsh
//  Singularity
//
//  Created by Jeffrey Bowles on 12/24/12.
//  Copyright (c) 2012 Jeffrey Bowles. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
