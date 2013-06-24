//
//  SGSettingsViewController.h
//  Singularity
//
//  Created by Jeffrey Bowles on 1/1/13.
//  Copyright (c) 2013 Jeffrey Bowles. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "SGViewController.h"
@interface SGSettingsViewController : UIViewController
@property (nonatomic, weak) id <ChangeParticleSystemSettings> delegate;
@end
