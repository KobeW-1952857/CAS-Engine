#include "utils/file_dialog.h"

#import <Cocoa/Cocoa.h>
#import <UniformTypeIdentifiers/UTType.h>

static NSArray<UTType *> *ParseFileTypeFilter(const char *filter) {
  if (!filter || strlen(filter) == 0) {
    return nil;
  }

  NSMutableArray<UTType *> *allowedTypes = [NSMutableArray array];
  NSString *filterString = [NSString stringWithUTF8String:filter];
  NSArray<NSString *> *extensions = [filterString componentsSeparatedByString:@","];

  for (NSString *ext in extensions) {
    NSString *trimmedExt = [ext stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
    UTType *type = [UTType typeWithFilenameExtension:trimmedExt];
    if (type) {
      [allowedTypes addObject:type];
    }
  }

  return [allowedTypes copy];
}

std::string FileDialog::openFile(const char *filter) {
  __block std::string openPath = std::string();

  void (^openPanelBlock)(void) = ^{
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.canChooseFiles = YES;
    panel.canChooseDirectories = NO;
    panel.allowsMultipleSelection = NO;

    NSArray<UTType *> *allowedTypes = ParseFileTypeFilter(filter);
    if (allowedTypes && allowedTypes.count > 0) {
      panel.allowedContentTypes = allowedTypes;
    }

    if ([panel runModal] == NSModalResponseOK) {
      NSURL *url = [panel URL];
      openPath = [[url path] UTF8String];
    } 
  };

  if ([NSThread isMainThread]) {
    openPanelBlock();
  } else {
    dispatch_sync(dispatch_get_main_queue(), openPanelBlock);
  }
  return openPath;
}

std::string FileDialog::saveFile(const char *filter) {
  __block std::string savePath = std::string();

  void (^savePanelBlock)(void) = ^{
    NSSavePanel *panel = [NSSavePanel savePanel];
    panel.canCreateDirectories = YES;

    NSArray<UTType *> *allowedTypes = ParseFileTypeFilter(filter);
    if (allowedTypes) {
      panel.allowedContentTypes = allowedTypes;
    }

    if ([panel runModal] == NSModalResponseOK) {
      NSURL *url = [panel URL];
      savePath = [[url path] UTF8String];
    }
  };

  if ([NSThread isMainThread]) {
    savePanelBlock();
  } else {
    dispatch_sync(dispatch_get_main_queue(), savePanelBlock);
  }
  return savePath;
}