/*
  ==============================================================================

   Preset Manager

  ==============================================================================
*/

#include "PresetManager.h"

namespace onsen
{
//==============================================================================

PresetManager::PresetManager (IAudioProcessorState* _processorState, juce::File _presetDir)
    : processorState (_processorState),
      presetDir (_presetDir),
      factoryPresetFiles(),
      userPresetFiles(),
      presetFiles(),
      currentPresetFile (getDefaultPresetFile())
{
    updateCurrentPresetBasedOnProcessorState();
}
//==============================================================================
void PresetManager::scanPresets()
{
    restorePresetFoldersAndPresetsIfNecessary();
    scanPresets (getFactoryPresetDir(), factoryPresetFiles);
    scanPresets (getUserPresetDir(), userPresetFiles);
    presetFiles.clear();
    presetFiles.add (getDefaultPresetFile());
    presetFiles.addArray (factoryPresetFiles);
    presetFiles.addArray (userPresetFiles);
}

juce::File PresetManager::getDefaultPresetFile()
{
    return getPresetDir().getChildFile ("Default.oapreset");
}

juce::File PresetManager::getFactoryPresetDir()
{
    return getPresetDir().getChildFile ("Factory");
}

juce::File PresetManager::getUserPresetDir()
{
    return getPresetDir().getChildFile ("User");
}

juce::Array<juce::File> PresetManager::getPresets()
{
    return presetFiles;
}

juce::Array<juce::File> PresetManager::getFactoryPresets()
{
    return factoryPresetFiles;
}

juce::Array<juce::File> PresetManager::getUserPresets()
{
    return userPresetFiles;
}

/*
    Save preset

    Folder should exist before calling it.
    */
void PresetManager::savePreset (juce::File file)
{
    if (file.getFullPathName() == "")
        return;

    // This check is necessary for file managers that can't force to
    // add the extension.
    if (! file.hasFileExtension (".oapreset"))
    {
        file = file.getParentDirectory().getChildFile (
            file.getFileName() + ".oapreset");
    }

    // TODO: Handle error causing while it's writing file
    auto presetRelativePath = file.getRelativePathFrom (getPresetDir());
    processorState->setPreset (presetRelativePath);

    auto state = processorState->copyState();
    std::unique_ptr<juce::XmlElement> stateXml (state.createXml());
    // Presets don't need <CurrentPreset> tag in the processor state
    stateXml->deleteAllChildElementsWithTagName ("CurrentPreset");

    auto presetXml = std::make_unique<juce::XmlElement> ("Preset");

    auto gadget = new juce::XmlElement ("Gadget");
    gadget->addTextElement ("OS-251");
    presetXml->addChildElement (gadget);

    auto savedByVersion = new juce::XmlElement ("SavedByVersion");
    savedByVersion->addTextElement (OS_251_PROJECT_VERSION);
    presetXml->addChildElement (savedByVersion);

    auto version = new juce::XmlElement ("Version");
    version->addTextElement ("0");
    presetXml->addChildElement (version);

    auto stateContainerXml = std::make_unique<juce::XmlElement> ("State");
    stateContainerXml->addChildElement (stateXml.release());
    presetXml->addChildElement (stateContainerXml.release());
    presetXml->writeTo (file);

    currentPresetFile = file;
}

/*
    Loads preset file. If the file is not found or valid, loads default
    */
void PresetManager::loadPreset (juce::File file)
{
    juce::XmlDocument xmlDocument (file);
    std::unique_ptr<juce::XmlElement> presetXml (xmlDocument.getDocumentElement());

    if (validatePresetXml (presetXml.get()))
    {
        loadPresetState (presetXml.get());
        auto presetRelativePath = file.getRelativePathFrom (getPresetDir());
        processorState->setPreset (presetRelativePath);
        currentPresetFile = file;
    }
    else
        loadDefaultFileSafely(); // TODO: change behavior?
}

juce::File PresetManager::getCurrentPresetFile()
{
    updateCurrentPresetBasedOnProcessorState();
    return currentPresetFile;
}

juce::String PresetManager::getPresetName (juce::File file)
{
    return file.getFileNameWithoutExtension();
}

void PresetManager::loadPrev()
{
    // TODO: If it's slow, Use hash table or binary search. Or cache
    //       currentPresetFile's index (Note that cached index can't be used
    //       after presetFiles is updated)
    int idx = presetFiles.indexOf (currentPresetFile);
    if (idx > 0)
        loadPreset (presetFiles[idx - 1]);
    else if (idx < 0) // Usually it doesn't happen
        loadPreset (getDefaultPresetFile());
}

void PresetManager::loadNext()
{
    int idx = presetFiles.indexOf (currentPresetFile);
    if (idx < presetFiles.size() - 1)
        loadPreset (presetFiles[idx + 1]);
    else if (idx < 0) // Usually it doesn't happen
        loadPreset (getDefaultPresetFile());
}

//==============================================================================

bool PresetManager::validatePresetFile (juce::File file)
{
    if (file.getFullPathName() == "" || ! file.existsAsFile())
        return false;
    juce::XmlDocument xmlDocument (file);
    std::unique_ptr<juce::XmlElement> presetXml (xmlDocument.getDocumentElement());

    return validatePresetXml (presetXml.get());
}

bool PresetManager::validatePresetXml (juce::XmlElement const* const presetXml)
{
    if (presetXml != nullptr
        && presetXml->hasTagName ("Preset")
        && presetXml->getChildByName ("Gadget") != nullptr
        && presetXml->getChildByName ("Gadget")->getFirstChildElement() != nullptr
        && presetXml->getChildByName ("Gadget")->getFirstChildElement()->isTextElement()
        && presetXml->getChildByName ("Gadget")->getFirstChildElement()->getText() == "OS-251"
        && presetXml->getChildByName ("SavedByVersion") != nullptr
        && presetXml->getChildByName ("SavedByVersion")->getFirstChildElement() != nullptr
        && presetXml->getChildByName ("SavedByVersion")->getFirstChildElement()->isTextElement()
        && presetXml->getChildByName ("Version") != nullptr
        && presetXml->getChildByName ("Version")->getFirstChildElement()->isTextElement()
        && presetXml->getChildByName ("Version")->getFirstChildElement()->getText() == "0"
        && presetXml->getChildByName ("State") != nullptr
        && presetXml->getChildByName ("State")->getChildByName (processorState->getProcessorName()) != nullptr)
        return true;

    return false;
}

void PresetManager::loadPresetState (juce::XmlElement const* const presetXml)
{
    processorState->replaceState (juce::ValueTree::fromXml (
        *(presetXml->getChildByName ("State")->getChildByName (processorState->getProcessorName()))));
}

void PresetManager::requireToUpdatePresetNameOnUI()
{
    if (onNeedToUpdateUI != nullptr)
        onNeedToUpdateUI();
}

void PresetManager::loadDefaultFileSafely()
{
    auto file = getDefaultPresetFile();
    juce::XmlDocument xmlDocument (getDefaultPresetFile());
    std::unique_ptr<juce::XmlElement> presetXml (xmlDocument.getDocumentElement());

    if (! validatePresetXml (presetXml.get()))
    {
        restoreDefaultPreset();

        // Retry to load the default preset.
        // To prevent infinate loop, I don't use recursive function call like
        // putting loadDefaultFileSafely() here.
        juce::XmlDocument newXmlDocument (getDefaultPresetFile());
        std::unique_ptr<juce::XmlElement> newPresetXml (xmlDocument.getDocumentElement());
        loadPresetState (newPresetXml.get());
        // TODO: Remove duplication. The following 2 lines appear repeatedly.
        auto presetRelativePath = file.getRelativePathFrom (getPresetDir());
        processorState->setPreset (presetRelativePath);
        currentPresetFile = file;
        return;
    }

    loadPresetState (presetXml.get());
    auto presetRelativePath = file.getRelativePathFrom (getPresetDir());
    processorState->setPreset (presetRelativePath);
    currentPresetFile = file;
}

juce::File PresetManager::getPresetDir()
{
    return presetDir;
}

juce::Array<juce::File> PresetManager::scanPresets (juce::File dir, juce::Array<juce::File>& presetFiles)
{
    dir.createDirectory(); // OK if it exists.
    auto files = dir.findChildFiles (juce::File::TypesOfFileToFind::findFiles, true, "*.oapreset");
    files.sort();

    return presetFiles = files;
}

void PresetManager::restorePresetFoldersAndPresetsIfNecessary()
{
    // Restore default preset if it doesn't exist.
    if (! getDefaultPresetFile().existsAsFile())
    {
        restoreDefaultPreset();
    }

    // Restore factory preset folder if it doesn't exist.
    if (! getFactoryPresetDir().exists() || getFactoryPresetDir().existsAsFile())
    {
        restoreFactoryPresets();
    }

    // Restore user preset folder if it doesn't exist.
    if (! getUserPresetDir().exists())
    {
        restoreUserPresetFolder();
    }
}

void PresetManager::restoreDefaultPreset()
{
    auto file = getDefaultPresetFile();

    // Try to delete a folder with the same name as default preset file.
    // It's OK to use for a file.
    file.deleteRecursively();
    // This line will create necessary sub folders
    file.create();
    juce::FileOutputStream fs (file);
    fs.write (BinaryData::Default_oapreset, BinaryData::Default_oapresetSize);
    fs.flush();
}

void PresetManager::restoreFactoryPresets()
{
    // Try to delete a file with the same name as factory preset folder
    // (Usually it doesn't happen).
    getFactoryPresetDir().deleteFile();

    auto dir = getFactoryPresetDir();
    for (auto& factoryPreset : factoryPresets)
    {
        auto dir = getFactoryPresetDir();
        auto file = dir.getChildFile (factoryPreset.path);
        // This line will create necessary sub folders
        file.create();
        juce::FileOutputStream fs (file);
        fs.write (factoryPreset.data, factoryPreset.size);
        fs.flush();
    }
}

void PresetManager::restoreUserPresetFolder()
{
    auto dir = getUserPresetDir();
    if (dir.exists())
        return;
    dir.createDirectory();
}

void PresetManager::updateCurrentPresetBasedOnProcessorState()
{
    auto relativePresetPath = processorState->getPreset();
    currentPresetFile = getPresetDir().getChildFile (relativePresetPath);
    if (! validatePresetFile (currentPresetFile))
    {
        currentPresetFile = "";
    }
}
} // namespace onsen
