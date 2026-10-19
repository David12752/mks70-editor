#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * ExportShare.h
 * Export and share patch features
 * 
 * Part of Improvement #20: Export/Share Features
 */

namespace Export
{

//==============================================================================
/**
 * Export format options
 */
enum class ExportFormat
{
    SysEx,          // Standard MIDI SysEx
    JSON,           // Human-readable JSON
    XMLPreset,      // JUCE ValueTree XML
    TextFile,       // Plain text parameter list
    HTMLReport,     // Visual HTML report
    PDFSheet        // Printable PDF (placeholder)
};

//==============================================================================
/**
 * Export options
 */
struct ExportOptions
{
    ExportFormat format = ExportFormat::SysEx;
    bool includeToneA = true;
    bool includeToneB = true;
    bool includePatch = true;
    bool includeMetadata = true;
    juce::String authorName;
    juce::String patchNotes;
};

//==============================================================================
/**
 * Exported patch data
 */
struct ExportedPatch
{
    juce::String name;
    juce::String format;
    juce::String content;
    juce::Time exportTime;
    size_t size = 0;
};

//==============================================================================
/**
 * Export Manager - handles all export operations
 */
class ExportManager
{
public:
    ExportManager() = default;
    
    // Export to file
    bool exportToFile(const juce::File& file, const ExportOptions& options, const juce::ValueTree& patchData)
    {
        juce::String content = generateExport(options, patchData);
        
        if (content.isEmpty())
            return false;
        
        return file.replaceWithText(content);
    }
    
    // Export to string (for clipboard, sharing)
    juce::String exportToString(const ExportOptions& options, const juce::ValueTree& patchData)
    {
        return generateExport(options, patchData);
    }
    
    // Copy to clipboard
    void copyToClipboard(const ExportOptions& options, const juce::ValueTree& patchData)
    {
        juce::String content = generateExport(options, patchData);
        juce::SystemClipboard::copyTextToClipboard(content);
    }
    
    // Get file extension for format
    static juce::String getExtension(ExportFormat format)
    {
        switch (format)
        {
            case ExportFormat::SysEx:      return ".syx";
            case ExportFormat::JSON:       return ".json";
            case ExportFormat::XMLPreset:  return ".xml";
            case ExportFormat::TextFile:   return ".txt";
            case ExportFormat::HTMLReport: return ".html";
            case ExportFormat::PDFSheet:   return ".pdf";
            default: return ".txt";
        }
    }
    
    // Get format name
    static juce::String getFormatName(ExportFormat format)
    {
        switch (format)
        {
            case ExportFormat::SysEx:      return "MIDI SysEx";
            case ExportFormat::JSON:       return "JSON";
            case ExportFormat::XMLPreset:  return "XML Preset";
            case ExportFormat::TextFile:   return "Text File";
            case ExportFormat::HTMLReport: return "HTML Report";
            case ExportFormat::PDFSheet:   return "PDF Sheet";
            default: return "Unknown";
        }
    }
    
private:
    juce::String generateExport(const ExportOptions& options, const juce::ValueTree& patchData)
    {
        switch (options.format)
        {
            case ExportFormat::SysEx:      return generateSysEx(options, patchData);
            case ExportFormat::JSON:       return generateJSON(options, patchData);
            case ExportFormat::XMLPreset:  return generateXML(options, patchData);
            case ExportFormat::TextFile:   return generateText(options, patchData);
            case ExportFormat::HTMLReport: return generateHTML(options, patchData);
            default: return {};
        }
    }
    
    juce::String generateSysEx(const ExportOptions& /*options*/, const juce::ValueTree& patchData)
    {
        // Generate MKS-70 SysEx format
        juce::MemoryOutputStream stream;
        
        // Roland SysEx header
        stream.writeByte(0xF0);  // SysEx start
        stream.writeByte(0x41);  // Roland ID
        stream.writeByte(0x36);  // MKS-70 ID
        stream.writeByte(0x00);  // Device ID
        
        // Write patch data
        for (int i = 0; i < patchData.getNumProperties(); i++)
        {
            auto propName = patchData.getPropertyName(i);
            int value = patchData.getProperty(propName);
            stream.writeByte(static_cast<uint8_t>(value & 0x7F));
        }
        
        // SysEx end
        stream.writeByte(0xF7);
        
        // Convert to hex string
        auto data = stream.getMemoryBlock();
        juce::String result;
        for (size_t i = 0; i < data.getSize(); i++)
        {
            result += juce::String::toHexString(static_cast<uint8_t>(data[i])).paddedLeft('0', 2);
            if (i < data.getSize() - 1)
                result += " ";
        }
        
        return result;
    }
    
    juce::String generateJSON(const ExportOptions& options, const juce::ValueTree& patchData)
    {
        juce::DynamicObject::Ptr root = new juce::DynamicObject();
        
        // Metadata
        if (options.includeMetadata)
        {
            juce::DynamicObject::Ptr meta = new juce::DynamicObject();
            meta->setProperty("name", patchData.getProperty("name", "Untitled"));
            meta->setProperty("author", options.authorName);
            meta->setProperty("notes", options.patchNotes);
            meta->setProperty("exportTime", juce::Time::getCurrentTime().toISO8601(true));
            meta->setProperty("format", "MKS-70");
            root->setProperty("metadata", juce::var(meta.get()));
        }
        
        // Patch parameters
        if (options.includePatch)
        {
            juce::DynamicObject::Ptr patch = new juce::DynamicObject();
            for (int i = 0; i < patchData.getNumProperties(); i++)
            {
                auto propName = patchData.getPropertyName(i);
                patch->setProperty(propName, patchData.getProperty(propName));
            }
            root->setProperty("patch", juce::var(patch.get()));
        }
        
        // Tone data
        if (options.includeToneA)
        {
            auto toneA = patchData.getChildWithName("ToneA");
            if (toneA.isValid())
            {
                juce::DynamicObject::Ptr tone = new juce::DynamicObject();
                for (int i = 0; i < toneA.getNumProperties(); i++)
                {
                    auto propName = toneA.getPropertyName(i);
                    tone->setProperty(propName, toneA.getProperty(propName));
                }
                root->setProperty("toneA", juce::var(tone.get()));
            }
        }
        
        if (options.includeToneB)
        {
            auto toneB = patchData.getChildWithName("ToneB");
            if (toneB.isValid())
            {
                juce::DynamicObject::Ptr tone = new juce::DynamicObject();
                for (int i = 0; i < toneB.getNumProperties(); i++)
                {
                    auto propName = toneB.getPropertyName(i);
                    tone->setProperty(propName, toneB.getProperty(propName));
                }
                root->setProperty("toneB", juce::var(tone.get()));
            }
        }
        
        return juce::JSON::toString(juce::var(root.get()), true);
    }
    
    juce::String generateXML(const ExportOptions& options, const juce::ValueTree& patchData)
    {
        juce::ValueTree export_("MKS70Export");
        
        if (options.includeMetadata)
        {
            juce::ValueTree meta("Metadata");
            meta.setProperty("author", options.authorName, nullptr);
            meta.setProperty("notes", options.patchNotes, nullptr);
            meta.setProperty("exportTime", juce::Time::getCurrentTime().toISO8601(true), nullptr);
            export_.appendChild(meta, nullptr);
        }
        
        if (options.includePatch || options.includeToneA || options.includeToneB)
        {
            auto patchCopy = patchData.createCopy();
            export_.appendChild(patchCopy, nullptr);
        }
        
        return export_.toXmlString();
    }
    
    juce::String generateText(const ExportOptions& options, const juce::ValueTree& patchData)
    {
        juce::String result;
        
        result += "MKS-70 Patch Export\n";
        result += "==================\n\n";
        
        if (options.includeMetadata)
        {
            result += "Patch Name: " + patchData.getProperty("name", "Untitled").toString() + "\n";
            result += "Author: " + options.authorName + "\n";
            result += "Export Date: " + juce::Time::getCurrentTime().toString(true, true) + "\n";
            
            if (options.patchNotes.isNotEmpty())
            {
                result += "\nNotes:\n";
                result += options.patchNotes + "\n";
            }
            
            result += "\n";
        }
        
        if (options.includePatch)
        {
            result += "PATCH PARAMETERS\n";
            result += "----------------\n";
            
            for (int i = 0; i < patchData.getNumProperties(); i++)
            {
                auto propName = patchData.getPropertyName(i);
                auto value = patchData.getProperty(propName);
                result += propName.toString() + ": " + value.toString() + "\n";
            }
            result += "\n";
        }
        
        if (options.includeToneA)
        {
            result += "TONE A PARAMETERS\n";
            result += "-----------------\n";
            auto toneA = patchData.getChildWithName("ToneA");
            if (toneA.isValid())
            {
                for (int i = 0; i < toneA.getNumProperties(); i++)
                {
                    auto propName = toneA.getPropertyName(i);
                    auto value = toneA.getProperty(propName);
                    result += propName.toString() + ": " + value.toString() + "\n";
                }
            }
            result += "\n";
        }
        
        if (options.includeToneB)
        {
            result += "TONE B PARAMETERS\n";
            result += "-----------------\n";
            auto toneB = patchData.getChildWithName("ToneB");
            if (toneB.isValid())
            {
                for (int i = 0; i < toneB.getNumProperties(); i++)
                {
                    auto propName = toneB.getPropertyName(i);
                    auto value = toneB.getProperty(propName);
                    result += propName.toString() + ": " + value.toString() + "\n";
                }
            }
        }
        
        return result;
    }
    
    juce::String generateHTML(const ExportOptions& options, const juce::ValueTree& patchData)
    {
        juce::String html;
        
        html += "<!DOCTYPE html>\n<html>\n<head>\n";
        html += "<title>MKS-70 Patch: " + patchData.getProperty("name", "Untitled").toString() + "</title>\n";
        html += "<style>\n";
        html += "body { font-family: 'Segoe UI', Arial, sans-serif; background: #1a1a2e; color: #eee; padding: 20px; }\n";
        html += ".container { max-width: 800px; margin: 0 auto; }\n";
        html += "h1 { color: #8b5cf6; border-bottom: 2px solid #8b5cf6; padding-bottom: 10px; }\n";
        html += "h2 { color: #a78bfa; margin-top: 30px; }\n";
        html += ".metadata { background: #2d2d44; padding: 15px; border-radius: 8px; margin-bottom: 20px; }\n";
        html += ".params { display: grid; grid-template-columns: repeat(2, 1fr); gap: 10px; }\n";
        html += ".param { background: #2d2d44; padding: 10px; border-radius: 4px; }\n";
        html += ".param-name { color: #a78bfa; font-size: 12px; }\n";
        html += ".param-value { color: #fff; font-size: 16px; font-weight: bold; }\n";
        html += "</style>\n</head>\n<body>\n<div class='container'>\n";
        
        html += "<h1>MKS-70 Patch: " + patchData.getProperty("name", "Untitled").toString() + "</h1>\n";
        
        if (options.includeMetadata)
        {
            html += "<div class='metadata'>\n";
            html += "<p><strong>Author:</strong> " + options.authorName + "</p>\n";
            html += "<p><strong>Export Date:</strong> " + juce::Time::getCurrentTime().toString(true, true) + "</p>\n";
            if (options.patchNotes.isNotEmpty())
                html += "<p><strong>Notes:</strong> " + options.patchNotes + "</p>\n";
            html += "</div>\n";
        }
        
        if (options.includePatch)
        {
            html += "<h2>Patch Parameters</h2>\n<div class='params'>\n";
            for (int i = 0; i < patchData.getNumProperties(); i++)
            {
                auto propName = patchData.getPropertyName(i);
                auto value = patchData.getProperty(propName);
                html += "<div class='param'><div class='param-name'>" + propName.toString() + "</div>";
                html += "<div class='param-value'>" + value.toString() + "</div></div>\n";
            }
            html += "</div>\n";
        }
        
        if (options.includeToneA)
        {
            html += "<h2>Tone A Parameters</h2>\n<div class='params'>\n";
            auto toneA = patchData.getChildWithName("ToneA");
            if (toneA.isValid())
            {
                for (int i = 0; i < toneA.getNumProperties(); i++)
                {
                    auto propName = toneA.getPropertyName(i);
                    auto value = toneA.getProperty(propName);
                    html += "<div class='param'><div class='param-name'>" + propName.toString() + "</div>";
                    html += "<div class='param-value'>" + value.toString() + "</div></div>\n";
                }
            }
            html += "</div>\n";
        }
        
        if (options.includeToneB)
        {
            html += "<h2>Tone B Parameters</h2>\n<div class='params'>\n";
            auto toneB = patchData.getChildWithName("ToneB");
            if (toneB.isValid())
            {
                for (int i = 0; i < toneB.getNumProperties(); i++)
                {
                    auto propName = toneB.getPropertyName(i);
                    auto value = toneB.getProperty(propName);
                    html += "<div class='param'><div class='param-name'>" + propName.toString() + "</div>";
                    html += "<div class='param-value'>" + value.toString() + "</div></div>\n";
                }
            }
            html += "</div>\n";
        }
        
        html += "</div>\n</body>\n</html>";
        
        return html;
    }
};

//==============================================================================
/**
 * Share Manager - handles sharing operations
 */
class ShareManager
{
public:
    ShareManager() = default;
    
    // Generate shareable link (mock - would integrate with cloud service)
    juce::String generateShareLink(const juce::ValueTree& patchData)
    {
        // In a real implementation, this would upload to a cloud service
        // For now, generate a fake link based on patch hash
        
        auto json = juce::JSON::toString(juce::var(patchData.toXmlString()));
        auto hash = juce::MD5(json.toUTF8()).toHexString().substring(0, 8);
        
        return "https://mks70patches.example.com/p/" + hash;
    }
    
    // Copy share text to clipboard
    void copyShareText(const juce::ValueTree& patchData)
    {
        juce::String shareText;
        shareText += "Check out my MKS-70 patch: ";
        shareText += patchData.getProperty("name", "Untitled").toString();
        shareText += "\n\n";
        shareText += generateShareLink(patchData);
        
        juce::SystemClipboard::copyTextToClipboard(shareText);
    }
};

//==============================================================================
/**
 * Export Panel - UI for export operations
 */
class ExportPanel : public juce::Component
{
public:
    ExportPanel()
    {
        // Format selector
        formatSelector.addItem("MIDI SysEx (.syx)", 1);
        formatSelector.addItem("JSON (.json)", 2);
        formatSelector.addItem("XML Preset (.xml)", 3);
        formatSelector.addItem("Text File (.txt)", 4);
        formatSelector.addItem("HTML Report (.html)", 5);
        formatSelector.setSelectedId(1);
        addAndMakeVisible(formatSelector);
        
        // Include checkboxes
        includeToneA.setButtonText("Include Tone A");
        includeToneA.setToggleState(true, juce::dontSendNotification);
        addAndMakeVisible(includeToneA);
        
        includeToneB.setButtonText("Include Tone B");
        includeToneB.setToggleState(true, juce::dontSendNotification);
        addAndMakeVisible(includeToneB);
        
        includePatch.setButtonText("Include Patch");
        includePatch.setToggleState(true, juce::dontSendNotification);
        addAndMakeVisible(includePatch);
        
        includeMetadata.setButtonText("Include Metadata");
        includeMetadata.setToggleState(true, juce::dontSendNotification);
        addAndMakeVisible(includeMetadata);
        
        // Author field
        authorField.setTextToShowWhenEmpty("Your name...", DS::Color::textDim);
        addAndMakeVisible(authorField);
        
        // Notes field
        notesField.setMultiLine(true);
        notesField.setTextToShowWhenEmpty("Patch notes...", DS::Color::textDim);
        addAndMakeVisible(notesField);
        
        // Export button
        exportButton.setButtonText("Export to File");
        exportButton.onClick = [this]() { exportToFile(); };
        addAndMakeVisible(exportButton);
        
        // Copy button
        copyButton.setButtonText("Copy to Clipboard");
        copyButton.onClick = [this]() { copyToClipboard(); };
        addAndMakeVisible(copyButton);
        
        // Share button
        shareButton.setButtonText("Generate Share Link");
        shareButton.onClick = [this]() { generateShareLink(); };
        addAndMakeVisible(shareButton);
    }
    
    void setPatchData(const juce::ValueTree& data)
    {
        patchData = data;
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(DS::Space::md);
        
        // Title
        bounds.removeFromTop(24);
        bounds.removeFromTop(DS::Space::sm);
        
        // Format selector
        auto row = bounds.removeFromTop(28);
        row.removeFromLeft(80); // Label space
        formatSelector.setBounds(row.removeFromLeft(200));
        bounds.removeFromTop(DS::Space::sm);
        
        // Checkboxes
        row = bounds.removeFromTop(24);
        includeToneA.setBounds(row.removeFromLeft(120));
        includeToneB.setBounds(row.removeFromLeft(120));
        bounds.removeFromTop(4);
        
        row = bounds.removeFromTop(24);
        includePatch.setBounds(row.removeFromLeft(120));
        includeMetadata.setBounds(row.removeFromLeft(140));
        bounds.removeFromTop(DS::Space::md);
        
        // Author
        row = bounds.removeFromTop(28);
        row.removeFromLeft(80);
        authorField.setBounds(row.removeFromLeft(200));
        bounds.removeFromTop(DS::Space::sm);
        
        // Notes
        bounds.removeFromTop(16); // Label
        notesField.setBounds(bounds.removeFromTop(80));
        bounds.removeFromTop(DS::Space::md);
        
        // Buttons
        row = bounds.removeFromTop(32);
        exportButton.setBounds(row.removeFromLeft(120));
        row.removeFromLeft(DS::Space::sm);
        copyButton.setBounds(row.removeFromLeft(140));
        row.removeFromLeft(DS::Space::sm);
        shareButton.setBounds(row.removeFromLeft(150));
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background
        g.setColour(DS::Color::surface);
        g.fillRoundedRectangle(bounds, DS::Radius::large);
        
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::large, 1.0f);
        
        auto contentBounds = bounds.reduced(DS::Space::md);
        
        // Title
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::heading2);
        g.drawText("Export & Share", contentBounds.removeFromTop(24), juce::Justification::centredLeft);
        
        contentBounds.removeFromTop(DS::Space::sm);
        
        // Format label
        g.setColour(DS::Color::textMuted);
        g.setFont(DS::Font::body);
        g.drawText("Format:", contentBounds.removeFromTop(28), juce::Justification::centredLeft);
        
        contentBounds.removeFromTop(DS::Space::sm);
        contentBounds.removeFromTop(24); // checkboxes
        contentBounds.removeFromTop(4);
        contentBounds.removeFromTop(24); // more checkboxes
        contentBounds.removeFromTop(DS::Space::md);
        
        // Author label
        g.drawText("Author:", contentBounds.removeFromTop(28), juce::Justification::centredLeft);
        contentBounds.removeFromTop(DS::Space::sm);
        
        // Notes label
        g.drawText("Notes:", contentBounds.removeFromTop(16), juce::Justification::centredLeft);
    }
    
private:
    void exportToFile()
    {
        auto options = getOptions();
        
        juce::FileChooser chooser("Export Patch",
                                  juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
                                  "*" + ExportManager::getExtension(options.format));
        
        if (chooser.browseForFileToSave(true))
        {
            auto file = chooser.getResult();
            
            if (exportManager.exportToFile(file, options, patchData))
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                    "Export Complete",
                    "Patch exported successfully to:\n" + file.getFullPathName());
            }
            else
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                    "Export Failed",
                    "Failed to export patch.");
            }
        }
    }
    
    void copyToClipboard()
    {
        auto options = getOptions();
        exportManager.copyToClipboard(options, patchData);
        
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
            "Copied",
            "Patch data copied to clipboard.");
    }
    
    void generateShareLink()
    {
        auto link = shareManager.generateShareLink(patchData);
        juce::SystemClipboard::copyTextToClipboard(link);
        
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
            "Share Link Generated",
            "Link copied to clipboard:\n" + link);
    }
    
    ExportOptions getOptions()
    {
        ExportOptions options;
        
        switch (formatSelector.getSelectedId())
        {
            case 1: options.format = ExportFormat::SysEx; break;
            case 2: options.format = ExportFormat::JSON; break;
            case 3: options.format = ExportFormat::XMLPreset; break;
            case 4: options.format = ExportFormat::TextFile; break;
            case 5: options.format = ExportFormat::HTMLReport; break;
            default: options.format = ExportFormat::SysEx; break;
        }
        
        options.includeToneA = includeToneA.getToggleState();
        options.includeToneB = includeToneB.getToggleState();
        options.includePatch = includePatch.getToggleState();
        options.includeMetadata = includeMetadata.getToggleState();
        options.authorName = authorField.getText();
        options.patchNotes = notesField.getText();
        
        return options;
    }
    
    ExportManager exportManager;
    ShareManager shareManager;
    juce::ValueTree patchData;
    
    juce::ComboBox formatSelector;
    juce::ToggleButton includeToneA;
    juce::ToggleButton includeToneB;
    juce::ToggleButton includePatch;
    juce::ToggleButton includeMetadata;
    juce::TextEditor authorField;
    juce::TextEditor notesField;
    juce::TextButton exportButton;
    juce::TextButton copyButton;
    juce::TextButton shareButton;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExportPanel)
};

} // namespace Export
