//
//  filechooser_mac.m
//  SpEd
//
//  Created by Denis Ivanov on 2015-04-30.
//  Copyright (c) 2015 DMD 'Ware. All rights reserved.
//

#include "../platform.h"
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

bool OpenFileDialog_Mac(char* initdir, char* filepath)
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:YES];
    [panel setAllowsMultipleSelection:NO]; // yes if more than one dir is allowed
    
    NSString* initdirns = [NSString stringWithUTF8String:initdir];
    NSURL* initurl = [NSURL URLWithString:[initdirns stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
    
    [panel setDirectoryURL:initurl];
    
    NSInteger clicked = [panel runModal];
    
    if (clicked == NSFileHandlingPanelOKButton)
    {
        for (NSURL *url in [panel URLs])
        {
            //NSString* nsstr = [url absoluteString];
            NSString* nsstr = [url path];
            strcpy(filepath, [nsstr UTF8String]);
            return true;
        }
    }
    
    return false;
}

bool SaveFileDialog_Mac(char* initdir, char* filepath)
{
    NSSavePanel *panel = [NSSavePanel savePanel];
    
    NSString* initdirns = [NSString stringWithUTF8String:initdir];
    NSURL* initurl = [NSURL URLWithString:[initdirns stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
    
    [panel setDirectoryURL:initurl];
    
    NSInteger clicked = [panel runModal];
    
    if (clicked == NSFileHandlingPanelOKButton)
    {
        NSURL *url = [panel URL];
        
        //NSString* nsstr = [url absoluteString];
        NSString* nsstr = [url path];
        strcpy(filepath, [nsstr UTF8String]);
        return true;
    }
    
    return false;
}