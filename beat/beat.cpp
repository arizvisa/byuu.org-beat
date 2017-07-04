#include <nall/nall.hpp>
#include <nall/beat/multi.hpp>
#include <nall/beat/archive.hpp>
#include <phoenix/phoenix.hpp>
using namespace nall;
using namespace phoenix;

#include "resource/resource.hpp"
#include "resource/resource.cpp"

const string Title = "beat v03";

struct Program : Window {
  VerticalLayout layout;
    HorizontalLayout actionLayout;
      Button applyButton;
      Button fileButton;
      Button pathButton;
      Button helpButton;
    HorizontalLayout controlLayout;
      Label optionsLabel;
      CheckButton manifestFlag;
      CheckButton deltaFlag;

  Program(int argc, char **argv) {
    string basepath = dir(realpath(argv[0]));
    bool helpExists = file::exists({basepath, "beat.html"});

    setTitle(Title);
    setResizable(false);

    layout.setMargin(5);
    applyButton.setImage({resource::apply, sizeof resource::apply});
    applyButton.setText("Apply Patch");
    fileButton.setImage({resource::file, sizeof resource::file});
    fileButton.setText("Create File Patch");
    pathButton.setImage({resource::path, sizeof resource::path});
    pathButton.setText("Create Folder Patch");
    helpButton.setImage({resource::help, sizeof resource::help});
    helpButton.setText("Help");
    optionsLabel.setText("Patch creation options:");
    manifestFlag.setText("Embed manifest");
    deltaFlag.setText("Delta mode");

    append(layout);
    layout.append(actionLayout, {0, 0}, 5);
      actionLayout.append(applyButton, {0, 0}, 5);
      actionLayout.append(fileButton, {0, 0}, 5);
      actionLayout.append(pathButton, {0, 0}, helpExists ? 5 : 0);
      if(helpExists) actionLayout.append(helpButton, {0, 0});
    layout.append(controlLayout, {0, 0});
      controlLayout.append(optionsLabel, {0, 0}, 5);
      controlLayout.append(manifestFlag, {0, 0}, 5);
      controlLayout.append(deltaFlag, {0, 0});

    onClose = &Application::quit;
    applyButton.onActivate = [=] { applyPatch(); };
    fileButton.onActivate = [=] { createFilePatch(); };
    pathButton.onActivate = [=] { createPathPatch(); };
    helpButton.onActivate = [=] { invoke("beat.html"); };

    Size desktopSize = Desktop::size();
    Size windowSize = layout.minimumSize();
    Position center = {
      (desktopSize.width  - windowSize.width ) / 2,
      (desktopSize.height - windowSize.height) / 2
    };
    setSmartGeometry({center, windowSize});
    setVisible();

    if(argc == 2) applyPatch(argv[1]);
  }

  void setBusy(bool busy) {
    if(busy == true) {
      setTitle({Title, " - Working"});
      layout.setEnabled(false);
    } else {
      setTitle(Title);
      layout.setEnabled(true);
    }
    Application::processEvents();
  }

  void applyPatch(string modifyName = "") {
    if(modifyName.empty()) modifyName = BrowserWindow().setParent(*this).setFilters("beat patches (*.bps,*.bpm)").open();
    if(modifyName.empty()) return;
    if(!file::exists(modifyName)) return;

    if(modifyName.iendswith(".bps")) {
      string sourceName = BrowserWindow().setParent(*this).setPath(dir(modifyName)).setFilters("All files (*)").open();
      if(sourceName.empty()) return;
      string targetName = BrowserWindow().setParent(*this).setPath(dir(sourceName)).setFilters("All files (*)").save();
      if(targetName.empty()) return;
      if(sourceName == targetName) {
        MessageWindow().setParent(*this).setText(
          "Source and target file names cannot be the same.\n"
          "You must select a new target file name."
        ).error();
        return;
      }

      setBusy(true);
      bpspatch patch;
      patch.modify(modifyName);
      patch.source(sourceName);
      patch.target(targetName);
      if(patch.apply() == bpspatch::result::success) {
        MessageWindow().setParent(*this).setText("Patch application was successful!").information();
      } else {
        MessageWindow().setParent(*this).setText("Patch application has failed!").error();
        file::remove(targetName);
      }
      setBusy(false);
    } else if(modifyName.iendswith(".bpm")) {
      string sourceName = BrowserWindow().setParent(*this).setPath(dir(modifyName)).directory();
      if(sourceName.empty()) return;
      string targetName = BrowserWindow().setParent(*this).setPath(parentdir(sourceName)).directory();
      if(targetName.empty()) return;
      if(sourceName == targetName) {
        MessageWindow().setParent(*this).setText(
          "Source and target folder names cannot be the same.\n"
          "You must select a new target folder name."
        ).error();
        return;
      }

      setBusy(true);
      bpsmulti patch;
      if(patch.apply(modifyName, sourceName, targetName)) {
        MessageWindow().setParent(*this).setText("Patch application was successful!").information();
      } else {
        MessageWindow().setParent(*this).setText("Patch application has failed!").information();
        directory::remove(targetName);
      }
      setBusy(false);
    }
  }

  void createFilePatch() {
    string sourceName = BrowserWindow().setParent(*this).setFilters("All files (*)").open();
    if(sourceName.empty()) return;
    string targetName = BrowserWindow().setParent(*this).setPath(dir(sourceName)).setFilters("All files (*)").open();
    if(targetName.empty()) return;
    if(sourceName == targetName) {
      MessageWindow().setParent(*this).setText("Source and target file names cannot be the same.\n").error();
      return;
    }
    string modifyName = BrowserWindow().setParent(*this).setPath(dir(targetName)).setFilters("beat patch (*.bps)").save();
    if(modifyName.empty()) return;
    string manifestName, manifestData;
    if(manifestFlag.checked()) {
      manifestName = BrowserWindow().setParent(*this).setPath(dir(modifyName)).setFilters("beat patch manifest (*.xml)").open();
      manifestData = string::read(manifestName);
    }

    setBusy(true);
    if(deltaFlag.checked() == false) {
      bpslinear patch;
      patch.source(sourceName);
      patch.target(targetName);
      if(patch.create(modifyName, manifestData)) {
        MessageWindow().setParent(*this).setText("Patch creation was successful!").information();
      } else {
        MessageWindow().setParent(*this).setText("Patch creation has failed!").error();
      }
    } else {
      bpsdelta patch;
      patch.source(sourceName);
      patch.target(targetName);
      if(patch.create(modifyName, manifestData)) {
        MessageWindow().setParent(*this).setText("Patch creation was successful!").information();
      } else {
        MessageWindow().setParent(*this).setText("Patch creation has failed!").error();
      }
    }
    setBusy(false);
  }

  void createPathPatch() {
    string sourceName = BrowserWindow().setParent(*this).directory();
    if(sourceName.empty()) return;
    string targetName = BrowserWindow().setParent(*this).setPath(parentdir(sourceName)).directory();
    if(targetName.empty()) return;
    if(sourceName == targetName) {
      MessageWindow().setParent(*this).setText("Source and target folder names cannot be the same.\n").error();
      return;
    }
    string modifyName = BrowserWindow().setParent(*this).setPath(parentdir(targetName)).setFilters("beat patch (*.bpm)").save();
    if(modifyName.empty()) return;
    string manifestName, manifestData;
    if(manifestFlag.checked()) {
      manifestName = BrowserWindow().setParent(*this).setPath(dir(modifyName)).setFilters("beat patch manifest (*.xml)").open();
      manifestData = string::read(manifestName);
    }

    setBusy(true);
    bpsmulti patch;
    if(patch.create(modifyName, sourceName, targetName, deltaFlag.checked(), manifestData)) {
      MessageWindow().setParent(*this).setText("Patch creation was successful!").information();
    } else {
      MessageWindow().setParent(*this).setText("Patch creation has failed!").error();
    }
    setBusy(false);
  }
};

int main(int argc, char **argv) {
  #if defined(PLATFORM_WINDOWS)
  utf8_args(argc, argv);
  #endif

  if(argc == 1 || (argc == 2 && file::exists(argv[1]))) {
    Program *program = new Program(argc, argv);
    Application::run();
    delete program;
    return 0;
  }

  lstring args;
  for(unsigned n = 0; n < argc; n++) args.append(argv[n]);

  bool create = false;
  bool apply = false;
  bool delta = false;
  string manifest;
  string source;
  string target;
  string modify;

  for(unsigned n = 1; n < args.size();) {
    if(0);

    else if(args[n] == "-encode") {
      create = true;
      n++;
    }

    else if(args[n] == "-decode") {
      apply = true;
      n++;
    }

    else if(args[n] == "-delta") {
      delta = true;
      n++;
    }

    else if(args[n] == "-linear") {
      delta = false;
      n++;
    }

    else if(args[n] == "-m" && n + 1 < args.size()) {
      manifest = file::read(args[n + 1]);
      n += 2;
    }

    else if(args[n] == "-t" && n + 1 < args.size()) {
      target = args[n + 1];
      n += 2;
    }

    else if(args[n] == "-p" && n + 1 < args.size()) {
      modify = args[n + 1];
      n += 2;
    }

    else {
      source = args[n];
      n++;
    }
  }

  //BPA - beat pack archive
  if(source.empty() && target.endswith("/") && !modify.empty()) {
    if(create) {
      beatArchive archive;
      bool result = archive.create(modify, target, string::read(manifest));
      if(result == false) print("error: archive creation failed\n");
      return 0;
    } else {
      beatArchive archive;
      bool result = archive.unpack(modify, target);
      if(result == false) print("error: archive extraction failed\n");
      return 0;
    }
  }

  //BPM - multi-file patch
  if(source.endswith("/") && target.endswith("/") && !modify.empty()) {
    if(create) {
      bpsmulti patch;
      bool result = patch.create(modify, source, target, delta, manifest);
      if(result == false) print("error: patch creation failed\n");
      return 0;
    }

    if(apply) {
      bpsmulti patch;
      bool result = patch.apply(modify, source, target);
      if(result == false) {
        directory::remove(target);
        print("error: patch application failed\n");
      }
      return 0;
    }
  }

  //BPS - single-file patch
  if(!source.endswith("/") && !target.endswith("/") && !modify.empty()) {
    if(create && delta == false) {
      bpslinear patch;
      patch.source(source);
      patch.target(target);
      bool result = patch.create(modify, manifest);
      if(result == false) print("error: patch creation failed\n");
      return 0;
    }

    if(create && delta == true) {
      bpsdelta patch;
      patch.source(source);
      patch.target(target);
      bool result = patch.create(modify, manifest);
      if(result == false) print("error: patch creation failed\n");
      return 0;
    }

    if(apply) {
      bpspatch patch;
      patch.modify(modify);
      patch.source(source);
      patch.target(target);
      bool result = patch.apply() == bpspatch::result::success;
      if(result == false) {
        file::remove(target);
        print("error: patch application failed\n");
      }
      return 0;
    }
  }

  print(Title, "\n");
  print("author: byuu\n");
  print("license: GPLv3\n\n");

  print("usage 1: beat -decode -p pack -t target [source]\n");
  print("usage 2: beat -encode [-delta] [-m manifest] -p pack -t target [source]\n");

  return 0;
}
