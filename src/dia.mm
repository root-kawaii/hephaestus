//
//  dia.cpp
//  DialogTstc++
//
//  Created by Frank Caggiano on 1/7/15.
//  Copyright (c) 2015 Frank Caggiano. All rights reserved.
//

#include "dia.h"
#include <iostream>
#import <Cocoa/Cocoa.h>

#include <string>

void pathos(std::string *ret) {
    NSArray *URLs;
    NSOpenPanel * panel = [NSOpenPanel openPanel];
    [panel setAllowsMultipleSelection:NO];
    [panel setResolvesAliases:YES];
    [panel setTreatsFilePackagesAsDirectories:YES];
    [panel setMessage:@"Please choose an image file."];
    
    [panel runModal];
    URLs = [panel URLs];

    NSMutableArray *URLStrings = [NSMutableArray array]; // This array will hold the URL strings

for (NSURL *url in URLs) {
    NSString *urlString = [url absoluteString];
    [URLStrings addObject:urlString];
}
    std::string bar = std::string([URLStrings[0] UTF8String]);
    size_t position = bar.find("assets/");   
    std::string newString = bar.substr(position);
    *ret = newString;
    
}