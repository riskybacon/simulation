//
//  SGAppDelegate.h
//  Singularity
//
//  Created by Jeffrey Bowles on 12/24/12.
//  Copyright (c) 2012 Jeffrey Bowles. All rights reserved.
//

#import <UIKit/UIKit.h>

@class SGViewController;
@class SGSettingsViewController;

@interface SGAppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) SGViewController* viewController;
@property (strong, nonatomic) SGSettingsViewController* settingsViewController;
@end
