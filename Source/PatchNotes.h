#pragma once

#include <JuceHeader.h>
#include "DesignSystem.h"

/**
 * PatchNotes.h
 * Notes and documentation system for patches
 * 
 * Part of Improvement #16: Patch Notes
 */

namespace Notes
{

//==============================================================================
/**
 * Patch note entry
 */
struct PatchNote
{
    juce::String patchName;
    juce::String notes;
    juce::StringArray tags;
    juce::Time created;
    juce::Time modified;
    int rating = 0;  // 0-5 stars
    bool isFavorite = false;
    
    juce::String author;
    juce::String category;
    juce::String usageHints;
};

//==============================================================================
/**
 * Notes Manager
 */
class NotesManager
{
public:
    NotesManager()
    {
        // Create notes directory
        notesDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                       .getChildFile("MKS70Editor")
                       .getChildFile("Notes");
        
        if (!notesDir.exists())
            notesDir.createDirectory();
        
        loadAllNotes();
    }
    
    void setNote(const juce::String& patchName, const PatchNote& note)
    {
        notes[patchName] = note;
        notes[patchName].modified = juce::Time::getCurrentTime();
        saveNote(patchName);
    }
    
    PatchNote* getNote(const juce::String& patchName)
    {
        auto it = notes.find(patchName);
        return it != notes.end() ? &it->second : nullptr;
    }
    
    bool hasNote(const juce::String& patchName) const
    {
        return notes.find(patchName) != notes.end();
    }
    
    void deleteNote(const juce::String& patchName)
    {
        notes.erase(patchName);
        
        juce::File noteFile = notesDir.getChildFile(sanitizeFilename(patchName) + ".json");
        noteFile.deleteFile();
    }
    
    void setRating(const juce::String& patchName, int rating)
    {
        if (notes.find(patchName) != notes.end())
        {
            notes[patchName].rating = juce::jlimit(0, 5, rating);
            notes[patchName].modified = juce::Time::getCurrentTime();
            saveNote(patchName);
        }
        else
        {
            // Create new note with just rating
            PatchNote note;
            note.patchName = patchName;
            note.rating = rating;
            note.created = juce::Time::getCurrentTime();
            note.modified = note.created;
            notes[patchName] = note;
            saveNote(patchName);
        }
    }
    
    void setFavorite(const juce::String& patchName, bool favorite)
    {
        if (notes.find(patchName) != notes.end())
        {
            notes[patchName].isFavorite = favorite;
            saveNote(patchName);
        }
        else
        {
            PatchNote note;
            note.patchName = patchName;
            note.isFavorite = favorite;
            note.created = juce::Time::getCurrentTime();
            note.modified = note.created;
            notes[patchName] = note;
            saveNote(patchName);
        }
    }
    
    std::vector<juce::String> getFavorites() const
    {
        std::vector<juce::String> favorites;
        for (const auto& pair : notes)
        {
            if (pair.second.isFavorite)
                favorites.push_back(pair.first);
        }
        return favorites;
    }
    
    std::vector<juce::String> getByRating(int minRating) const
    {
        std::vector<juce::String> result;
        for (const auto& pair : notes)
        {
            if (pair.second.rating >= minRating)
                result.push_back(pair.first);
        }
        return result;
    }
    
private:
    void loadAllNotes()
    {
        for (auto& file : notesDir.findChildFiles(juce::File::findFiles, false, "*.json"))
        {
            auto json = juce::JSON::parse(file);
            if (json.isObject())
            {
                PatchNote note;
                note.patchName = json.getProperty("patchName", "").toString();
                note.notes = json.getProperty("notes", "").toString();
                note.author = json.getProperty("author", "").toString();
                note.category = json.getProperty("category", "").toString();
                note.usageHints = json.getProperty("usageHints", "").toString();
                note.rating = static_cast<int>(json.getProperty("rating", 0));
                note.isFavorite = static_cast<bool>(json.getProperty("isFavorite", false));
                note.created = juce::Time(static_cast<juce::int64>(json.getProperty("created", 0)));
                note.modified = juce::Time(static_cast<juce::int64>(json.getProperty("modified", 0)));
                
                if (auto* tagsArray = json.getProperty("tags", juce::var()).getArray())
                {
                    for (const auto& tag : *tagsArray)
                        note.tags.add(tag.toString());
                }
                
                if (note.patchName.isNotEmpty())
                    notes[note.patchName] = note;
            }
        }
    }
    
    void saveNote(const juce::String& patchName)
    {
        if (notes.find(patchName) == notes.end())
            return;
        
        const auto& note = notes[patchName];
        
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();
        obj->setProperty("patchName", note.patchName);
        obj->setProperty("notes", note.notes);
        obj->setProperty("author", note.author);
        obj->setProperty("category", note.category);
        obj->setProperty("usageHints", note.usageHints);
        obj->setProperty("rating", note.rating);
        obj->setProperty("isFavorite", note.isFavorite);
        obj->setProperty("created", note.created.toMilliseconds());
        obj->setProperty("modified", note.modified.toMilliseconds());
        
        juce::Array<juce::var> tagsArray;
        for (const auto& tag : note.tags)
            tagsArray.add(tag);
        obj->setProperty("tags", tagsArray);
        
        juce::File noteFile = notesDir.getChildFile(sanitizeFilename(patchName) + ".json");
        noteFile.replaceWithText(juce::JSON::toString(juce::var(obj.get())));
    }
    
    juce::String sanitizeFilename(const juce::String& name)
    {
        return name.replaceCharacters(" /\\:*?\"<>|", "___________");
    }
    
    juce::File notesDir;
    std::map<juce::String, PatchNote> notes;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NotesManager)
};

//==============================================================================
/**
 * Star Rating Component
 */
class StarRating : public juce::Component
{
public:
    StarRating()
    {
        setRepaintsOnMouseActivity(true);
    }
    
    void setRating(int r) { rating = juce::jlimit(0, 5, r); repaint(); }
    int getRating() const { return rating; }
    
    std::function<void(int)> onRatingChanged;
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float starSize = juce::jmin(bounds.getWidth() / 5.0f, bounds.getHeight()) - 2;
        
        float x = bounds.getX();
        float y = bounds.getCentreY() - starSize / 2;
        
        for (int i = 0; i < 5; i++)
        {
            bool filled = i < rating;
            bool hovered = isMouseOver() && i < getStarAtPosition(getMouseXYRelative().toFloat());
            
            juce::Colour starColor;
            if (filled)
                starColor = juce::Colour(0xFFFFD700);  // Gold
            else if (hovered)
                starColor = juce::Colour(0xFFFFD700).withAlpha(0.5f);
            else
                starColor = DS::Color::textMuted.withAlpha(0.3f);
            
            g.setColour(starColor);
            drawStar(g, x + i * (starSize + 2), y, starSize, filled);
        }
    }
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        int newRating = getStarAtPosition(e.position);
        if (newRating == rating)
            newRating = 0;  // Click same star to clear
        
        setRating(newRating);
        
        if (onRatingChanged)
            onRatingChanged(rating);
    }
    
private:
    int getStarAtPosition(juce::Point<float> pos) const
    {
        auto bounds = getLocalBounds().toFloat();
        float starSize = juce::jmin(bounds.getWidth() / 5.0f, bounds.getHeight()) - 2;
        
        int star = static_cast<int>((pos.x - bounds.getX()) / (starSize + 2)) + 1;
        return juce::jlimit(1, 5, star);
    }
    
    void drawStar(juce::Graphics& g, float x, float y, float size, bool filled)
    {
        juce::Path star;
        float cx = x + size / 2;
        float cy = y + size / 2;
        float outerR = size / 2;
        float innerR = outerR * 0.4f;
        
        for (int i = 0; i < 5; i++)
        {
            float outerAngle = juce::MathConstants<float>::pi * (i * 0.4f - 0.5f);
            float innerAngle = outerAngle + juce::MathConstants<float>::pi * 0.2f;
            
            if (i == 0)
                star.startNewSubPath(cx + outerR * std::cos(outerAngle), 
                                     cy + outerR * std::sin(outerAngle));
            else
                star.lineTo(cx + outerR * std::cos(outerAngle), 
                           cy + outerR * std::sin(outerAngle));
            
            star.lineTo(cx + innerR * std::cos(innerAngle), 
                       cy + innerR * std::sin(innerAngle));
        }
        star.closeSubPath();
        
        if (filled)
            g.fillPath(star);
        else
            g.strokePath(star, juce::PathStrokeType(1.0f));
    }
    
    int rating = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StarRating)
};

//==============================================================================
/**
 * Patch Notes Editor Panel
 */
class NotesEditor : public juce::Component
{
public:
    NotesEditor(NotesManager& manager) : notesManager(manager)
    {
        // Notes text editor
        notesTextEditor.setMultiLine(true);
        notesTextEditor.setReturnKeyStartsNewLine(true);
        notesTextEditor.setScrollbarsShown(true);
        notesTextEditor.setPopupMenuEnabled(true);
        addAndMakeVisible(notesTextEditor);
        
        // Author field
        authorLabel.setText("Author:", juce::dontSendNotification);
        authorLabel.setFont(DS::Font::label);
        authorLabel.setColour(juce::Label::textColourId, DS::Color::textMuted);
        addAndMakeVisible(authorLabel);
        addAndMakeVisible(authorEditor);
        
        // Tags field
        tagsLabel.setText("Tags:", juce::dontSendNotification);
        tagsLabel.setFont(DS::Font::label);
        tagsLabel.setColour(juce::Label::textColourId, DS::Color::textMuted);
        addAndMakeVisible(tagsLabel);
        addAndMakeVisible(tagsEditor);
        
        // Star rating
        addAndMakeVisible(starRating);
        starRating.onRatingChanged = [this](int rating)
        {
            if (currentPatchName.isNotEmpty())
                notesManager.setRating(currentPatchName, rating);
        };
        
        // Favorite button
        favoriteButton.setButtonText("Fav");
        favoriteButton.onClick = [this]
        {
            if (currentPatchName.isNotEmpty())
            {
                isFavorite = !isFavorite;
                notesManager.setFavorite(currentPatchName, isFavorite);
                updateFavoriteButton();
            }
        };
        addAndMakeVisible(favoriteButton);
        
        // Save button
        saveButton.setButtonText("Save Notes");
        saveButton.onClick = [this] { saveCurrentNotes(); };
        addAndMakeVisible(saveButton);
    }
    
    void loadPatch(const juce::String& patchName)
    {
        currentPatchName = patchName;
        
        if (auto* note = notesManager.getNote(patchName))
        {
            notesTextEditor.setText(note->notes);
            authorEditor.setText(note->author);
            tagsEditor.setText(note->tags.joinIntoString(", "));
            starRating.setRating(note->rating);
            isFavorite = note->isFavorite;
        }
        else
        {
            notesTextEditor.clear();
            authorEditor.clear();
            tagsEditor.clear();
            starRating.setRating(0);
            isFavorite = false;
        }
        
        updateFavoriteButton();
        repaint();
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(DS::Space::md);
        
        // Header
        auto headerRow = bounds.removeFromTop(30);
        favoriteButton.setBounds(headerRow.removeFromRight(30));
        starRating.setBounds(headerRow.removeFromRight(120));
        bounds.removeFromTop(DS::Space::sm);
        
        // Author
        auto authorRow = bounds.removeFromTop(24);
        authorLabel.setBounds(authorRow.removeFromLeft(50));
        authorEditor.setBounds(authorRow);
        bounds.removeFromTop(DS::Space::sm);
        
        // Tags
        auto tagsRow = bounds.removeFromTop(24);
        tagsLabel.setBounds(tagsRow.removeFromLeft(50));
        tagsEditor.setBounds(tagsRow);
        bounds.removeFromTop(DS::Space::sm);
        
        // Save button
        saveButton.setBounds(bounds.removeFromBottom(30));
        bounds.removeFromBottom(DS::Space::sm);
        
        // Notes editor
        notesTextEditor.setBounds(bounds);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        g.setColour(DS::Color::surface);
        g.fillRoundedRectangle(bounds, DS::Radius::medium);
        
        g.setColour(DS::Color::border);
        g.drawRoundedRectangle(bounds, DS::Radius::medium, 1.0f);
        
        // Title
        g.setColour(DS::Color::text);
        g.setFont(DS::Font::heading);
        g.drawText("PATCH NOTES: " + currentPatchName, 
                   bounds.reduced(DS::Space::md).removeFromTop(24), 
                   juce::Justification::centredLeft);
    }
    
private:
    void saveCurrentNotes()
    {
        if (currentPatchName.isEmpty())
            return;
        
        PatchNote note;
        note.patchName = currentPatchName;
        note.notes = notesTextEditor.getText();
        note.author = authorEditor.getText();
        note.rating = starRating.getRating();
        note.isFavorite = isFavorite;
        note.created = juce::Time::getCurrentTime();
        
        // Parse tags
        juce::String tagsText = tagsEditor.getText();
        note.tags.addTokens(tagsText, ",", "");
        for (auto& tag : note.tags)
            tag = tag.trim();
        
        notesManager.setNote(currentPatchName, note);
    }
    
    void updateFavoriteButton()
    {
        favoriteButton.setColour(juce::TextButton::textColourOnId, 
                                 isFavorite ? juce::Colour(0xFFFFD700) : DS::Color::textMuted);
        favoriteButton.setColour(juce::TextButton::textColourOffId, 
                                 isFavorite ? juce::Colour(0xFFFFD700) : DS::Color::textMuted);
    }
    
    NotesManager& notesManager;
    juce::String currentPatchName;
    bool isFavorite = false;
    
    juce::TextEditor notesTextEditor;
    juce::Label authorLabel;
    juce::TextEditor authorEditor;
    juce::Label tagsLabel;
    juce::TextEditor tagsEditor;
    StarRating starRating;
    juce::TextButton favoriteButton;
    juce::TextButton saveButton;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NotesEditor)
};

} // namespace Notes
