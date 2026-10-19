#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * BackupRestore.h
 * Full backup and restore system for synth state
 * 
 * Part of Improvement #12: Backup & Restore
 */

namespace Backup
{

//==============================================================================
/**
 * Backup entry metadata
 */
struct BackupInfo
{
    juce::String filename;
    juce::String displayName;
    juce::Time timestamp;
    juce::String description;
    int numPatches = 0;
    int numTones = 0;
    bool isAutoBackup = false;
    juce::int64 fileSize = 0;
};

//==============================================================================
/**
 * Backup Manager
 */
class BackupManager
{
public:
    BackupManager()
    {
        // Create backup directory
        backupDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                        .getChildFile("MKS70Editor")
                        .getChildFile("Backups");
        
        if (!backupDir.exists())
            backupDir.createDirectory();
    }
    
    //==========================================================================
    // Backup Operations
    //==========================================================================
    
    bool createBackup(const juce::String& name, const juce::String& description,
                      const juce::MemoryBlock& data)
    {
        // Generate filename with timestamp
        juce::Time now = juce::Time::getCurrentTime();
        juce::String timestamp = now.formatted("%Y%m%d_%H%M%S");
        juce::String safeName = name.replaceCharacters(" /\\:*?\"<>|", "___________");
        juce::String filename = timestamp + "_" + safeName + ".mks70backup";
        
        juce::File backupFile = backupDir.getChildFile(filename);
        
        // Create backup structure
        juce::MemoryOutputStream out;
        
        // Header
        out.writeString("MKS70BACKUP");
        out.writeInt(1); // Version
        out.writeString(name);
        out.writeString(description);
        out.writeInt64(now.toMilliseconds());
        
        // Data
        out.writeInt(static_cast<int>(data.getSize()));
        out.write(data.getData(), data.getSize());
        
        // Write to file
        if (backupFile.replaceWithData(out.getData(), out.getDataSize()))
        {
            if (onBackupCreated)
                onBackupCreated(backupFile.getFileName());
            return true;
        }
        
        return false;
    }
    
    bool createAutoBackup(const juce::MemoryBlock& data)
    {
        // Keep max 10 auto backups
        cleanupAutoBackups(10);
        
        return createBackup("AutoBackup", "Automatic backup", data);
    }
    
    bool restoreBackup(const juce::String& filename, juce::MemoryBlock& outData)
    {
        juce::File backupFile = backupDir.getChildFile(filename);
        
        if (!backupFile.existsAsFile())
            return false;
        
        juce::MemoryBlock fileData;
        if (!backupFile.loadFileAsData(fileData))
            return false;
        
        juce::MemoryInputStream in(fileData, false);
        
        // Verify header
        if (in.readString() != "MKS70BACKUP")
            return false;
        
        int version = in.readInt();
        if (version != 1)
            return false;
        
        // Skip metadata
        in.readString(); // name
        in.readString(); // description
        in.readInt64();  // timestamp
        
        // Read data
        int dataSize = in.readInt();
        outData.setSize(dataSize);
        in.read(outData.getData(), dataSize);
        
        if (onBackupRestored)
            onBackupRestored(filename);
        
        return true;
    }
    
    bool deleteBackup(const juce::String& filename)
    {
        juce::File backupFile = backupDir.getChildFile(filename);
        return backupFile.deleteFile();
    }
    
    //==========================================================================
    // Backup Listing
    //==========================================================================
    
    std::vector<BackupInfo> listBackups()
    {
        std::vector<BackupInfo> backups;
        
        for (auto& file : backupDir.findChildFiles(juce::File::findFiles, false, "*.mks70backup"))
        {
            BackupInfo info;
            info.filename = file.getFileName();
            info.fileSize = file.getSize();
            info.timestamp = file.getLastModificationTime();
            
            // Try to read metadata
            juce::MemoryBlock fileData;
            if (file.loadFileAsData(fileData))
            {
                juce::MemoryInputStream in(fileData, false);
                
                if (in.readString() == "MKS70BACKUP")
                {
                    in.readInt(); // version
                    info.displayName = in.readString();
                    info.description = in.readString();
                    info.timestamp = juce::Time(in.readInt64());
                    info.isAutoBackup = info.displayName.startsWith("AutoBackup");
                }
            }
            
            if (info.displayName.isEmpty())
                info.displayName = file.getFileNameWithoutExtension();
            
            backups.push_back(info);
        }
        
        // Sort by timestamp (newest first)
        std::sort(backups.begin(), backups.end(),
            [](const BackupInfo& a, const BackupInfo& b)
            {
                return a.timestamp > b.timestamp;
            });
        
        return backups;
    }
    
    int getBackupCount()
    {
        return backupDir.getNumberOfChildFiles(juce::File::findFiles, "*.mks70backup");
    }
    
    juce::int64 getTotalBackupSize()
    {
        juce::int64 total = 0;
        for (auto& file : backupDir.findChildFiles(juce::File::findFiles, false, "*.mks70backup"))
            total += file.getSize();
        return total;
    }
    
    //==========================================================================
    // Cleanup
    //==========================================================================
    
    void cleanupAutoBackups(int keepCount)
    {
        auto backups = listBackups();
        
        int autoCount = 0;
        for (const auto& backup : backups)
        {
            if (backup.isAutoBackup)
            {
                autoCount++;
                if (autoCount > keepCount)
                    deleteBackup(backup.filename);
            }
        }
    }
    
    void deleteOldBackups(int daysOld)
    {
        juce::Time cutoff = juce::Time::getCurrentTime() - juce::RelativeTime::days(daysOld);
        
        for (auto& file : backupDir.findChildFiles(juce::File::findFiles, false, "*.mks70backup"))
        {
            if (file.getLastModificationTime() < cutoff)
                file.deleteFile();
        }
    }
    
    //==========================================================================
    // Callbacks
    //==========================================================================
    
    std::function<void(const juce::String&)> onBackupCreated;
    std::function<void(const juce::String&)> onBackupRestored;
    
private:
    juce::File backupDir;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BackupManager)
};

//==============================================================================
/**
 * Backup Panel UI
 */
class BackupPanel : public juce::Component, public juce::ListBoxModel
{
public:
    BackupPanel(BackupManager& manager) : backupManager(manager)
    {
        refreshBackupList();
        
        listBox.setModel(this);
        listBox.setRowHeight(50);
        addAndMakeVisible(listBox);
        
        // Create backup button
        createButton.setButtonText("Create Backup");
        createButton.onClick = [this] { showCreateDialog(); };
        addAndMakeVisible(createButton);
        
        // Restore button
        restoreButton.setButtonText("Restore");
        restoreButton.onClick = [this] { restoreSelected(); };
        restoreButton.setEnabled(false);
        addAndMakeVisible(restoreButton);
        
        // Delete button
        deleteButton.setButtonText("Delete");
        deleteButton.onClick = [this] { deleteSelected(); };
        deleteButton.setEnabled(false);
        addAndMakeVisible(deleteButton);
        
        // Refresh button
        refreshButton.setButtonText("Refresh");
        refreshButton.onClick = [this] { refreshBackupList(); };
        addAndMakeVisible(refreshButton);
    }
    
    void refreshBackupList()
    {
        backups = backupManager.listBackups();
        listBox.updateContent();
        updateStats();
        repaint();
    }
    
    std::function<void(const juce::String&)> onRestoreRequested;
    std::function<juce::MemoryBlock()> getCurrentStateForBackup;
    
    //==========================================================================
    // ListBoxModel
    //==========================================================================
    
    int getNumRows() override { return static_cast<int>(backups.size()); }
    
    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (row >= static_cast<int>(backups.size()))
            return;
        
        const auto& backup = backups[row];
        
        if (rowIsSelected)
            g.fillAll(DS::Color::accent.withAlpha(0.2f));
        else if (row % 2)
            g.fillAll(DS::Color::surfaceAlt.withAlpha(0.3f));
        
        auto bounds = juce::Rectangle<int>(0, 0, width, height).reduced(8, 4);
        
        // Icon
        g.setColour(backup.isAutoBackup ? DS::Color::textMuted : DS::Color::accent);
        g.fillRoundedRectangle(bounds.removeFromLeft(36).toFloat().reduced(4), 4);
        
        bounds.removeFromLeft(8);
        
        // Name and description
        auto textBounds = bounds.removeFromLeft(width - 150);
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::body.boldened());
        g.drawText(backup.displayName, textBounds.removeFromTop(20), juce::Justification::centredLeft);
        
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::label);
        g.drawText(backup.description, textBounds, juce::Justification::centredLeft);
        
        // Date and size
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::label);
        g.drawText(backup.timestamp.toString(true, true, false), 
                   bounds.removeFromTop(20), juce::Justification::centredRight);
        g.drawText(formatFileSize(backup.fileSize), bounds, juce::Justification::centredRight);
    }
    
    void selectedRowsChanged(int lastRowSelected) override
    {
        bool hasSelection = lastRowSelected >= 0;
        restoreButton.setEnabled(hasSelection);
        deleteButton.setEnabled(hasSelection);
    }
    
    //==========================================================================
    // Layout
    //==========================================================================
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(DS::Space::md);
        
        // Stats area
        statsArea = bounds.removeFromTop(50);
        bounds.removeFromTop(DS::Space::sm);
        
        // Buttons
        auto buttonArea = bounds.removeFromBottom(35);
        createButton.setBounds(buttonArea.removeFromLeft(120).reduced(2));
        buttonArea.removeFromLeft(DS::Space::sm);
        restoreButton.setBounds(buttonArea.removeFromLeft(80).reduced(2));
        deleteButton.setBounds(buttonArea.removeFromLeft(80).reduced(2));
        refreshButton.setBounds(buttonArea.removeFromRight(80).reduced(2));
        
        bounds.removeFromBottom(DS::Space::sm);
        
        // List
        listBox.setBounds(bounds);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        g.setColour(DS::Color::surface);
        g.fillRoundedRectangle(bounds, DS::Radius::medium);
        
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::medium, 1.0f);
        
        // Stats
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::heading);
        g.drawText("Backups", statsArea.removeFromLeft(100), juce::Justification::centredLeft);
        
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::body);
        g.drawText(juce::String(backups.size()) + " backups, " + formatFileSize(totalSize),
                   statsArea, juce::Justification::centredRight);
    }
    
private:
    void showCreateDialog()
    {
        auto* dialog = new juce::AlertWindow("Create Backup", 
                                              "Enter a name for this backup:",
                                              juce::MessageBoxIconType::NoIcon);
        
        dialog->addTextEditor("name", "My Backup", "Name:");
        dialog->addTextEditor("desc", "", "Description (optional):");
        dialog->addButton("Create", 1);
        dialog->addButton("Cancel", 0);
        
        dialog->enterModalState(true, juce::ModalCallbackFunction::create(
            [this, dialog](int result)
            {
                if (result == 1 && getCurrentStateForBackup)
                {
                    juce::String name = dialog->getTextEditorContents("name");
                    juce::String desc = dialog->getTextEditorContents("desc");
                    juce::MemoryBlock data = getCurrentStateForBackup();
                    
                    if (backupManager.createBackup(name, desc, data))
                        refreshBackupList();
                }
                delete dialog;
            }));
    }
    
    void restoreSelected()
    {
        int selected = listBox.getSelectedRow();
        if (selected >= 0 && selected < static_cast<int>(backups.size()))
        {
            if (onRestoreRequested)
                onRestoreRequested(backups[selected].filename);
        }
    }
    
    void deleteSelected()
    {
        int selected = listBox.getSelectedRow();
        if (selected >= 0 && selected < static_cast<int>(backups.size()))
        {
            if (juce::AlertWindow::showOkCancelBox(
                    juce::MessageBoxIconType::WarningIcon,
                    "Delete Backup",
                    "Are you sure you want to delete this backup?"))
            {
                backupManager.deleteBackup(backups[selected].filename);
                refreshBackupList();
            }
        }
    }
    
    void updateStats()
    {
        totalSize = backupManager.getTotalBackupSize();
    }
    
    juce::String formatFileSize(juce::int64 bytes)
    {
        if (bytes < 1024)
            return juce::String(bytes) + " B";
        if (bytes < 1024 * 1024)
            return juce::String(bytes / 1024) + " KB";
        return juce::String(bytes / (1024 * 1024)) + " MB";
    }
    
    BackupManager& backupManager;
    std::vector<BackupInfo> backups;
    
    juce::ListBox listBox;
    juce::TextButton createButton;
    juce::TextButton restoreButton;
    juce::TextButton deleteButton;
    juce::TextButton refreshButton;
    
    juce::Rectangle<int> statsArea;
    juce::int64 totalSize = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BackupPanel)
};

} // namespace Backup
