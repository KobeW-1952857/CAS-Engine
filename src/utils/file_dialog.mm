#include "utils/file_dialog.h"

#include <Cocoa/Cocoa.h>

std::string FileDialog::openFile(const char *filter) {
  std::string openPath = std::string();

  NSOpenPanel *panel = [NSOpenPanel openPanel];
  [panel setCanChooseFiles:YES];
  [panel setCanChooseDirectories:NO];
  [panel setAllowsMultipleSelection:NO];
  [panel setAllowedContentTypes:@[]];

  NSInteger result = [panel runModal];
  if (result == NSModalResponseOK) {
    NSURL *url = [panel URL];
    openPath = [[url path] UTF8String];
  }

  return openPath;
}

std::string FileDialog::saveFile(const char *filter) {
  std::string savePath = std::string();

  NSSavePanel *panel = [NSSavePanel savePanel];

  NSInteger result = [panel runModal];
  if (result == NSModalResponseOK) {
    NSURL *url = [panel URL];
    savePath = [[url path] UTF8String];
  }
  return savePath;
}