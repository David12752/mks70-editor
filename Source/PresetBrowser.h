/*
  ==============================================================================

    PresetBrowser.h
    Advanced preset browser with search, tags, and favorites
    
    Features:
    - Full-text search
    - Category tags (Bass, Pad, Lead, etc.)
    - Favorites system
    - Sort by name/date/author
    - Preview on hover

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

//==============================================================================
/**
    PresetTag - Represents a category/tag for presets
*/
enum class PresetTag
{
    None = 0,
    Bass,
    Pad,
    Lead,
    Keys,
    Strings,
    Brass,
    Bells,
    FX,
    Drums,
    Synth,
    Organ,
    Other,
    
    NumTags
};

inline juce::String getTagName(PresetTag tag)
{
    switch (tag)
    {
        case PresetTag::Bass:    return "Bass";
        case PresetTag::Pad:     return "Pad";
        case PresetTag::Lead:    return "Lead";
        case PresetTag::Keys:    return "Keys";
        case PresetTag::Strings: return "Strings";
        case PresetTag::Brass:   return "Brass";
        case PresetTag::Bells:   return "Bells";
        case PresetTag::FX:      return "FX";
        case PresetTag::Drums:   return "Drums";
        case PresetTag::Synth:   return "Synth";
        case PresetTag::Organ:   return "Organ";
        case PresetTag::Other:   return "Other";
        default:                 return "";
    }
}

inline juce::Colour getTagColor(PresetTag tag)
{
    switch (tag)
    {
        case PresetTag::Bass:    return juce::Colour(0xFF3B82F6);  // Blue
        case PresetTag::Pad:     return juce::Colour(0xFF8B5CF6);  // Purple
        case PresetTag::Lead:    return juce::Colour(0xFFEC4899);  // Pink
        case PresetTag::Keys:    return juce::Colour(0xFF10B981);  // Green
        case PresetTag::Strings: return juce::Colour(0xFFF59E0B);  // Amber
        case PresetTag::Brass:   return juce::Colour(0xFFEF4444);  // Red
        case PresetTag::Bells:   return juce::Colour(0xFF22D3EE);  // Cyan
        case PresetTag::FX:      return juce::Colour(0xFF6366F1);  // Indigo
        case PresetTag::Drums:   return juce::Colour(0xFF14B8A6);  // Teal
        case PresetTag::Synth:   return juce::Colour(0xFFD946EF);  // Fuchsia
        case PresetTag::Organ:   return juce::Colour(0xFF84CC16);  // Lime
        default:                 return juce::Colour(0xFF64748B);  // Gray
    }
}

//==============================================================================
/**
    PresetInfo - Metadata for a preset
*/
struct PresetInfo
{
    juce::String name;
    juce::String author;
    juce::String description;
    juce::File file;
    PresetTag tag = PresetTag::None;
    bool isFavorite = false;
    juce::Time dateModified;
    int rating = 0;  // 0-5 stars
    
    // Additional tags (multiple tags per preset)
    std::vector<PresetTag> additionalTags;
    
    bool matchesSearch(const juce::String& query) const
    {
        if (query.isEmpty()) return true;
        
        auto lowerQuery = query.toLowerCase();
        return name.toLowerCase().contains(lowerQuery) ||
               author.toLowerCase().contains(lowerQuery) ||
               description.toLowerCase().contains(lowerQuery) ||
               getTagName(tag).toLowerCase().contains(lowerQuery);
    }
    
    bool hasTag(PresetTag t) const
    {
        if (tag == t) return true;
        for (auto at : additionalTags)
            if (at == t) return true;
        return false;
    }
};

//==============================================================================
/**
    PresetDatabase - Manages all presets with search and filter
*/
class PresetDatabase
{
public:
    PresetDatabase()
    {
        // Will scan folders on initialization
    }
    
    void scanFolder(const juce::File& folder)
    {
        if (!folder.isDirectory()) return;
        
        auto files = folder.findChildFiles(juce::File::findFiles, true, "*.syx");
        
        for (auto& file : files)
        {
            PresetInfo info;
            info.name = file.getFileNameWithoutExtension();
            info.file = file;
            info.dateModified = file.getLastModificationTime();
            
            // Auto-tag based on folder name
            auto parentName = file.getParentDirectory().getFileName().toLowerCase();
            info.tag = guessTagFromFolder(parentName);
            
            // Try to load author from metadata file
            auto metaFile = file.withFileExtension("json");
            if (metaFile.existsAsFile())
            {
                loadMetadata(info, metaFile);
            }
            
            presets.push_back(info);
        }
        
        sortByName();
    }
    
    PresetTag guessTagFromFolder(const juce::String& folderName)
    {
        if (folderName.contains("bass"))    return PresetTag::Bass;
        if (folderName.contains("pad"))     return PresetTag::Pad;
        if (folderName.contains("lead"))    return PresetTag::Lead;
        if (folderName.contains("key"))     return PresetTag::Keys;
        if (folderName.contains("string"))  return PresetTag::Strings;
        if (folderName.contains("brass"))   return PresetTag::Brass;
        if (folderName.contains("bell"))    return PresetTag::Bells;
        if (folderName.contains("fx"))      return PresetTag::FX;
        if (folderName.contains("drum"))    return PresetTag::Drums;
        if (folderName.contains("synth"))   return PresetTag::Synth;
        if (folderName.contains("organ"))   return PresetTag::Organ;
        return PresetTag::Other;
    }
    
    void loadMetadata(PresetInfo& info, const juce::File& metaFile)
    {
        auto json = juce::JSON::parse(metaFile);
        if (json.isObject())
        {
            info.author = json.getProperty("author", "").toString();
            info.description = json.getProperty("description", "").toString();
            info.rating = static_cast<int>(json.getProperty("rating", 0));
            info.isFavorite = static_cast<bool>(json.getProperty("favorite", false));
        }
    }
    
    void saveMetadata(const PresetInfo& info)
    {
        auto metaFile = info.file.withFileExtension("json");
        
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();
        obj->setProperty("author", info.author);
        obj->setProperty("description", info.description);
        obj->setProperty("rating", info.rating);
        obj->setProperty("favorite", info.isFavorite);
        obj->setProperty("tag", static_cast<int>(info.tag));
        
        metaFile.replaceWithText(juce::JSON::toString(obj.get()));
    }
    
    // Search and filter
    std::vector<PresetInfo*> search(const juce::String& query, 
                                    PresetTag filterTag = PresetTag::None,
                                    bool favoritesOnly = false)
    {
        std::vector<PresetInfo*> results;
        
        for (auto& preset : presets)
        {
            // Filter by favorites
            if (favoritesOnly && !preset.isFavorite)
                continue;
            
            // Filter by tag
            if (filterTag != PresetTag::None && !preset.hasTag(filterTag))
                continue;
            
            // Filter by search query
            if (!preset.matchesSearch(query))
                continue;
            
            results.push_back(&preset);
        }
        
        return results;
    }
    
    // Sorting
    void sortByName()
    {
        std::sort(presets.begin(), presets.end(),
                  [](const PresetInfo& a, const PresetInfo& b) {
                      return a.name.compareIgnoreCase(b.name) < 0;
                  });
    }
    
    void sortByDate()
    {
        std::sort(presets.begin(), presets.end(),
                  [](const PresetInfo& a, const PresetInfo& b) {
                      return a.dateModified > b.dateModified;  // Newest first
                  });
    }
    
    void sortByRating()
    {
        std::sort(presets.begin(), presets.end(),
                  [](const PresetInfo& a, const PresetInfo& b) {
                      return a.rating > b.rating;  // Highest first
                  });
    }
    
    // Accessors
    const std::vector<PresetInfo>& getAllPresets() const { return presets; }
    int getPresetCount() const { return static_cast<int>(presets.size()); }
    
    PresetInfo* getPreset(int index)
    {
        if (index >= 0 && index < static_cast<int>(presets.size()))
            return &presets[index];
        return nullptr;
    }
    
    // Favorites
    void toggleFavorite(int index)
    {
        if (auto* preset = getPreset(index))
        {
            preset->isFavorite = !preset->isFavorite;
            saveMetadata(*preset);
        }
    }
    
    void setRating(int index, int rating)
    {
        if (auto* preset = getPreset(index))
        {
            preset->rating = juce::jlimit(0, 5, rating);
            saveMetadata(*preset);
        }
    }
    
private:
    std::vector<PresetInfo> presets;
};

//==============================================================================
/**
    TagButton - Button for filtering by tag
*/
class TagButton : public juce::TextButton
{
public:
    TagButton(PresetTag t) : tag(t)
    {
        setButtonText(getTagName(tag));
        setClickingTogglesState(true);
        
        auto color = getTagColor(tag);
        setColour(juce::TextButton::buttonColourId, color.withAlpha(0.2f));
        setColour(juce::TextButton::buttonOnColourId, color);
        setColour(juce::TextButton::textColourOffId, color);
        setColour(juce::TextButton::textColourOnId, DS::Color::textPrimary());
    }
    
    PresetTag getTag() const { return tag; }
    
private:
    PresetTag tag;
};

//==============================================================================
/**
    SearchBox - Search input with clear button
*/
class SearchBox : public juce::Component,
                  public juce::TextEditor::Listener
{
public:
    SearchBox()
    {
        searchIcon.setText(juce::CharPointer_UTF8("\xF0\x9F\x94\x8D"), juce::dontSendNotification);  // 🔍
        searchIcon.setFont(DS::Font::normal());
        searchIcon.setColour(juce::Label::textColourId, DS::Color::textMuted());
        addAndMakeVisible(searchIcon);
        
        editor.setTextToShowWhenEmpty("Search presets...", DS::Color::textMuted());
        editor.setFont(DS::Font::normal());
        editor.setColour(juce::TextEditor::backgroundColourId, DS::Color::bgInput());
        editor.setColour(juce::TextEditor::outlineColourId, DS::Color::border());
        editor.setColour(juce::TextEditor::textColourId, DS::Color::textPrimary());
        editor.addListener(this);
        addAndMakeVisible(editor);
        
        clearButton.setButtonText(juce::CharPointer_UTF8("\xE2\x9C\x95"));  // ✕
        clearButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        clearButton.setColour(juce::TextButton::textColourOffId, DS::Color::textMuted());
        clearButton.onClick = [this]() { 
            editor.clear(); 
            if (onSearchChanged) onSearchChanged("");
        };
        addAndMakeVisible(clearButton);
        clearButton.setVisible(false);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        searchIcon.setBounds(bounds.removeFromLeft(30));
        clearButton.setBounds(bounds.removeFromRight(30));
        editor.setBounds(bounds);
    }
    
    void textEditorTextChanged(juce::TextEditor&) override
    {
        auto text = editor.getText();
        clearButton.setVisible(text.isNotEmpty());
        if (onSearchChanged)
            onSearchChanged(text);
    }
    
    juce::String getText() const { return editor.getText(); }
    void clear() { editor.clear(); }
    
    std::function<void(const juce::String&)> onSearchChanged;
    
private:
    juce::Label searchIcon;
    juce::TextEditor editor;
    juce::TextButton clearButton;
};

//==============================================================================
/**
    PresetListItem - Single preset in the list
*/
class PresetListItem : public juce::Component
{
public:
    PresetListItem(PresetInfo* info, bool selected = false)
        : preset(info), isSelected(selected)
    {
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background
        if (isSelected)
            g.setColour(DS::Color::Accent::primary().withAlpha(0.3f));
        else if (isMouseOver())
            g.setColour(DS::Color::bgHover());
        else
            g.setColour(DS::Color::bgCard());
        
        g.fillRoundedRectangle(bounds.reduced(2), DS::Radius::S);
        
        // Favorite star
        if (preset && preset->isFavorite)
        {
            g.setColour(juce::Colour(0xFFF59E0B));
            g.setFont(DS::Font::normal());
            g.drawText(juce::CharPointer_UTF8("\xE2\x98\x85"), bounds.removeFromLeft(24), 
                       juce::Justification::centred);  // ★
        }
        else
        {
            bounds.removeFromLeft(24);
        }
        
        if (preset)
        {
            // Tag badge
            auto tagBadge = bounds.removeFromLeft(60).reduced(4);
            g.setColour(getTagColor(preset->tag));
            g.fillRoundedRectangle(tagBadge, DS::Radius::XS);
            g.setColour(DS::Color::textPrimary());
            g.setFont(DS::Font::tiny());
            g.drawText(getTagName(preset->tag), tagBadge, juce::Justification::centred);
            
            bounds.removeFromLeft(8);
            
            // Name
            g.setColour(DS::Color::textPrimary());
            g.setFont(DS::Font::normal());
            g.drawText(preset->name, bounds.removeFromLeft(200), juce::Justification::centredLeft);
            
            // Author
            if (preset->author.isNotEmpty())
            {
                g.setColour(DS::Color::textSecondary());
                g.setFont(DS::Font::small());
                g.drawText(preset->author, bounds.removeFromLeft(100), juce::Justification::centredLeft);
            }
            
            // Rating stars
            bounds = getLocalBounds().toFloat();
            auto ratingArea = bounds.removeFromRight(80);
            g.setColour(DS::Color::textMuted());
            g.setFont(DS::Font::small());
            juce::String stars;
            for (int i = 0; i < 5; ++i)
                stars += (i < preset->rating) ? juce::CharPointer_UTF8("\xE2\x98\x85") : juce::CharPointer_UTF8("\xE2\x98\x86");
            g.drawText(stars, ratingArea, juce::Justification::centredRight);
        }
    }
    
    void setSelected(bool sel) { isSelected = sel; repaint(); }
    
    PresetInfo* getPreset() const { return preset; }
    
private:
    PresetInfo* preset;
    bool isSelected;
};

//==============================================================================
/**
    EnhancedPresetBrowser - Main preset browser component
*/
class EnhancedPresetBrowser : public juce::Component
{
public:
    EnhancedPresetBrowser()
    {
        // Search box
        addAndMakeVisible(searchBox);
        searchBox.onSearchChanged = [this](const juce::String& query) {
            currentSearch = query;
            refreshList();
        };
        
        // Tag filter buttons
        for (int i = 1; i < static_cast<int>(PresetTag::NumTags); ++i)
        {
            auto* btn = new TagButton(static_cast<PresetTag>(i));
            btn->onClick = [this, btn]() {
                // Toggle filter
                if (btn->getToggleState())
                    currentTagFilter = btn->getTag();
                else
                    currentTagFilter = PresetTag::None;
                
                // Deselect other tag buttons
                for (auto* other : tagButtons)
                    if (other != btn)
                        other->setToggleState(false, juce::dontSendNotification);
                
                refreshList();
            };
            tagButtons.add(btn);
            addAndMakeVisible(btn);
        }
        
        // Favorites filter
        favoritesButton.setButtonText(juce::CharPointer_UTF8("\xE2\x98\x85 Favorites"));
        favoritesButton.setClickingTogglesState(true);
        favoritesButton.setColour(juce::TextButton::buttonColourId, DS::Color::bgInput());
        favoritesButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFF59E0B));
        favoritesButton.onClick = [this]() {
            showFavoritesOnly = favoritesButton.getToggleState();
            refreshList();
        };
        addAndMakeVisible(favoritesButton);
        
        // Sort combo
        sortCombo.addItem("Name", 1);
        sortCombo.addItem("Date", 2);
        sortCombo.addItem("Rating", 3);
        sortCombo.setSelectedId(1);
        sortCombo.onChange = [this]() {
            switch (sortCombo.getSelectedId())
            {
                case 1: database.sortByName(); break;
                case 2: database.sortByDate(); break;
                case 3: database.sortByRating(); break;
            }
            refreshList();
        };
        addAndMakeVisible(sortCombo);
        
        // Preset list viewport
        listViewport.setViewedComponent(&listContent, false);
        listViewport.setScrollBarsShown(true, false);
        addAndMakeVisible(listViewport);
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(DS::Color::bg());
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(DS::Space::M);
        
        // Search bar
        searchBox.setBounds(bounds.removeFromTop(36));
        bounds.removeFromTop(DS::Space::M);
        
        // Tag filter row
        auto tagRow = bounds.removeFromTop(30);
        int tagWidth = 65;
        for (auto* btn : tagButtons)
        {
            btn->setBounds(tagRow.removeFromLeft(tagWidth));
            tagRow.removeFromLeft(4);
        }
        
        bounds.removeFromTop(DS::Space::S);
        
        // Favorites + Sort row
        auto filterRow = bounds.removeFromTop(30);
        favoritesButton.setBounds(filterRow.removeFromLeft(100));
        filterRow.removeFromLeft(DS::Space::L);
        sortCombo.setBounds(filterRow.removeFromRight(120));
        
        bounds.removeFromTop(DS::Space::M);
        
        // Preset list
        listViewport.setBounds(bounds);
        
        layoutList();
    }
    
    void scanPresets(const juce::File& folder)
    {
        database.scanFolder(folder);
        refreshList();
    }
    
    void refreshList()
    {
        filteredPresets = database.search(currentSearch, currentTagFilter, showFavoritesOnly);
        layoutList();
    }
    
    std::function<void(PresetInfo*)> onPresetSelected;
    std::function<void(PresetInfo*)> onPresetDoubleClicked;
    
private:
    PresetDatabase database;
    
    SearchBox searchBox;
    juce::OwnedArray<TagButton> tagButtons;
    juce::TextButton favoritesButton;
    juce::ComboBox sortCombo;
    
    juce::Viewport listViewport;
    juce::Component listContent;
    juce::OwnedArray<PresetListItem> listItems;
    
    std::vector<PresetInfo*> filteredPresets;
    juce::String currentSearch;
    PresetTag currentTagFilter = PresetTag::None;
    bool showFavoritesOnly = false;
    int selectedIndex = -1;
    
    void layoutList()
    {
        listItems.clear();
        
        int y = 0;
        const int itemHeight = 36;
        const int width = listViewport.getWidth() - listViewport.getScrollBarThickness();
        
        for (size_t i = 0; i < filteredPresets.size(); ++i)
        {
            auto* item = new PresetListItem(filteredPresets[i], static_cast<int>(i) == selectedIndex);
            item->setBounds(0, y, width, itemHeight);
            item->addMouseListener(this, false);
            listItems.add(item);
            listContent.addAndMakeVisible(item);
            y += itemHeight + 2;
        }
        
        listContent.setSize(width, juce::jmax(y, listViewport.getHeight()));
    }
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        for (int i = 0; i < listItems.size(); ++i)
        {
            if (listItems[i]->contains(e.getEventRelativeTo(listItems[i]).getPosition()))
            {
                selectedIndex = i;
                
                // Update selection visual
                for (int j = 0; j < listItems.size(); ++j)
                    listItems[j]->setSelected(j == i);
                
                if (onPresetSelected)
                    onPresetSelected(filteredPresets[i]);
                
                break;
            }
        }
    }
    
    void mouseDoubleClick(const juce::MouseEvent& e) override
    {
        for (int i = 0; i < listItems.size(); ++i)
        {
            if (listItems[i]->contains(e.getEventRelativeTo(listItems[i]).getPosition()))
            {
                if (onPresetDoubleClicked)
                    onPresetDoubleClicked(filteredPresets[i]);
                break;
            }
        }
    }
};
