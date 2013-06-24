//
//  SGViewController.m
//  Singularity
//
//  Created by Jeffrey Bowles on 12/24/12.
//  Copyright (c) 2012 Jeffrey Bowles. All rights reserved.
//

#import "SGViewController.h"
#import "SGSettingsViewController.h"
#include "OpenGL.h"
#include "Scene.h"

@interface SGViewController () {
   Scene* _scene;
   GLKView* _view;
}
@property (strong, nonatomic) EAGLContext *context;
@property (strong, nonatomic) IBOutlet UISlider* singularitySlider;
@property BOOL pauseSimulation;
- (void)setupGL;
- (void)tearDownGL;
- (void)initScene;
- (void)addGestureRecognizers:(UIView*) view;
- (void)willAnimateRotationToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation duration:(NSTimeInterval)duration;
- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation duration:(NSTimeInterval)duration;
- (void)printOrientation:(UIInterfaceOrientation)interfaceOrientation;
- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation;
@end

@implementation SGViewController
@synthesize singularitySlider;
@synthesize pauseSimulation;

- (void)initScene
{
   // Get the size of the main screen. These values tend to be
   // iPhone 3 / 3gs:   320 x 480    non-retina
   // iPhone 4 / 4s:    640 x 960    retina
   // iPhone 5:         640 x 1136   retina
   // iPad 1,2:         768 x 1024   non-retina
   // iPad 3:          1536 x 2048   retina
   UIScreen* mainscr = [UIScreen mainScreen];
   unsigned int width = mainscr.currentMode.size.width;
	unsigned int height = mainscr.currentMode.size.height;
   
   NSString* resourcePath = [[NSBundle mainBundle] resourcePath];

   // Check to see the device is rotated and in landscape mode. If so,
   // switch height and width
   if(UIInterfaceOrientationIsLandscape(self.interfaceOrientation))
   {
      unsigned int tmpHeight = height;
      height = width;
      width = tmpHeight;
   }
   _scene = new Scene(std::string([resourcePath UTF8String]), width, height);
   
   singularitySlider.value = _scene->getSingularityMass();
   singularitySlider.hidden = YES;
   
   pauseSimulation = NO;
}

- (void)viewDidLoad
{
   [super viewDidLoad];
   
   self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
   
   if (!self.context) {
      NSLog(@"Failed to create ES context");
   }
   
   _view = (GLKView *)self.view;
   _view.context = self.context;
   _view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
   _view.multipleTouchEnabled = YES;
   
   [self addGestureRecognizers:_view];
   
   [self setupGL];
}

// adds a set of gesture recognizers to one of our piece subviews
- (void)addGestureRecognizers:(UIView*) view
{
   UIPinchGestureRecognizer *pinchGesture = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(handlePinchGesture:)];
   [pinchGesture setDelegate:self];
   [view addGestureRecognizer:pinchGesture];
   
   
   UIPanGestureRecognizer *panGesture     = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(handlePanGesture:)];
   [panGesture setDelegate:self];
   [view addGestureRecognizer:panGesture];
   
   UITapGestureRecognizer* tapGesture     = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleTapGesture:)];
   [tapGesture setDelegate:self];
   [view addGestureRecognizer:tapGesture];
}

- (void)dealloc
{    
   [self tearDownGL];
   
   if ([EAGLContext currentContext] == self.context) {
      [EAGLContext setCurrentContext:nil];
   }
}

- (void)didReceiveMemoryWarning
{
   [super didReceiveMemoryWarning];
   
   if ([self isViewLoaded] && ([[self view] window] == nil)) {
      self.view = nil;
      
      [self tearDownGL];
      
      if ([EAGLContext currentContext] == self.context) {
         [EAGLContext setCurrentContext:nil];
      }
      self.context = nil;
   }
   
   // Dispose of any resources that can be recreated.
}

- (void)setupGL
{
   [EAGLContext setCurrentContext:self.context];
   [self initScene];
}

- (void)tearDownGL
{
   [EAGLContext setCurrentContext:self.context];
   delete _scene;
}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update
{
   if(pauseSimulation == NO)
   {
      _scene->update();
   }
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
   _scene->draw();
}

- (void)handlePinchGesture:(UIPinchGestureRecognizer *)gestureRecognizer
{
   CGFloat scale    = [gestureRecognizer scale];
   CGFloat velocity = [gestureRecognizer velocity];
   
   _scene->pinchGesture(scale, velocity);
}

- (void)handlePanGesture:(UIPanGestureRecognizer *)gestureRecognizer
{
   CGPoint translation = [gestureRecognizer translationInView:_view];
   _scene->panGesture(translation.x, translation.y);
   [gestureRecognizer setTranslation:CGPointMake(0, 0) inView:_view];
}

- (void)handleTapGesture:(UITapGestureRecognizer *)gestureRecognizer
{
   CGPoint pos = [gestureRecognizer locationInView:_view];
   _scene->tapGesture(pos.x, pos.y);
   
   if(_scene->settingsTapped()) {
      NSLog(@"Settings tapped top level");
   }
}

- (void)printOrientation:(UIInterfaceOrientation) interfaceOrientation
{
   switch(interfaceOrientation)
   {
      case UIInterfaceOrientationPortrait:
         std::cerr << "portrait";
         break;
         
      case UIInterfaceOrientationPortraitUpsideDown:
         std::cerr << "portrait upside down";
         break;
         
      case UIInterfaceOrientationLandscapeLeft:
         std::cerr << "landscape left";
         break;
         
      case UIInterfaceOrientationLandscapeRight:
         std::cerr << "landscape right";
         break;
         
      default:
         std::cerr << "Unknown" << std::endl;
         break;
   }
   
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation duration:(NSTimeInterval)duration
{
}

- (void)willAnimateRotationToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation duration:(NSTimeInterval)duration;
{
   UIScreen* mainscr = [UIScreen mainScreen];
   unsigned int width = mainscr.currentMode.size.width;
	unsigned int height = mainscr.currentMode.size.height;

   if(UIInterfaceOrientationIsLandscape(self.interfaceOrientation))
   {
      _scene->resize(height, width);
   }
   else
   {
      _scene->resize(width, height);
   }
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
}

- (void) prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
   if([segue.identifier isEqualToString:@"Settings"])
   {
      SGSettingsViewController* settingsViewController = segue.destinationViewController;
      settingsViewController.delegate = self;
   }
   singularitySlider.hidden = YES;
}


-(float) getSingularityMass
{
   return _scene->getSingularityMass();
}

-(float) getParticleMass
{
   return _scene->getParticleMass();
}

-(void)  setSingularityMass:(float)mass
{
   _scene->setSingularityMass(mass);
}

-(void) setParticleMass:(float)mass
{
   _scene->setParticleMass(mass);
}

-(void) doneChangingSettings
{
   [self dismissViewControllerAnimated:YES completion:nil];
}

- (IBAction)changeSingularityMass:(UISlider *)sender
{
   // Get the value of the slider
   float mass = [sender value];
   // Update the particle system with the new mass
   _scene->setSingularityMass(mass);
}

- (IBAction)settingsButtonPressed:(id)sender
{
   if(singularitySlider.isHidden)
   {
      singularitySlider.hidden = NO;
   }
   else
   {
      singularitySlider.hidden = YES;
   }
}
- (IBAction)pauseButton:(id)sender
{
   if(pauseSimulation == YES)
   {
      pauseSimulation = NO;
   }
   else
   {
      pauseSimulation = YES;
   }
}

@end
