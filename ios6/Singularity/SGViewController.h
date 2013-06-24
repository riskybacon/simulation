//
//  SGViewController.h
//  Singularity
//
//  Created by Jeffrey Bowles on 12/24/12.
//  Copyright (c) 2012 Jeffrey Bowles. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

@protocol ChangeParticleSystemSettings <NSObject>
-(float) getSingularityMass;
-(float) getParticleMass;
-(void)  setSingularityMass:(float)mass;
-(void)  setParticleMass:(float)mass;
-(void)  doneChangingSettings;
@end

@interface SGViewController : GLKViewController <UIGestureRecognizerDelegate, ChangeParticleSystemSettings>
-(IBAction)handlePinchGesture:(UIPinchGestureRecognizer*)recognizer;
-(IBAction)handlePanGesture:(UIPanGestureRecognizer*)recognizer;
-(IBAction)handleTapGesture:(UITapGestureRecognizer*)recognizer;
-(float)getSingularityMass;
-(float)getParticleMass;
-(void)setSingularityMass:(float)mass;
-(void)setParticleMass:(float)mass;
-(void)doneChangingSettings;
@end
