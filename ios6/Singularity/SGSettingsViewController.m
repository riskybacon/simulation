//
//  SGSettingsViewController.m
//  Singularity
//
//  Created by Jeffrey Bowles on 1/1/13.
//  Copyright (c) 2013 Jeffrey Bowles. All rights reserved.
//

#import "SGSettingsViewController.h"

@interface SGSettingsViewController ()
@property (strong, nonatomic) IBOutlet UILabel* singularityValueLabel;
@property (strong, nonatomic) IBOutlet UISlider* singularityValueSlider;
- (IBAction)singularityMassChanged:(id)sender;
- (void) setSingularityValueLabelText:(float)value;
@end

@implementation SGSettingsViewController

@synthesize singularityValueLabel;
@synthesize singularityValueSlider;

@synthesize delegate;

- (void) setSingularityValueLabelText:(float)value {
   NSString* text = [[NSString alloc] initWithFormat:@"%f kg", value, nil];
   self.singularityValueLabel.text = text;
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
   NSLog(@"initWithNibName");
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
   [super viewDidLoad];
   // Do any additional setup after loading the view from its nib.
   float singularityMass = [delegate getSingularityMass];
   [singularityValueSlider setValue:singularityMass];
   [self setSingularityValueLabelText:singularityMass];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)singularityMassChanged:(id)sender
{
   // Get the value of the slider
   float mass = [(UISlider*) sender value];
   // Update the label
   [self setSingularityValueLabelText:mass];
   // Update the particle system with the new mass
   [delegate setSingularityMass:mass];
}

- (IBAction)done:(id)sender
{
   [delegate doneChangingSettings];
}

@end
