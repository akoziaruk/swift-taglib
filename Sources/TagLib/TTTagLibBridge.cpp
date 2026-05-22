#include "fileref.h"
#include "tpropertymap.h"
#include "tvariant.h"
#include "tbytevector.h"

#include <cstdlib>
#include <cstring>
#include <string>

extern "C" {

struct TTTagLibTags {
  char *title;
  char *artist;
  char *album;
  char *albumArtist;
  char *genre;
  int hasYear;
  int year;
  int hasTrackNumber;
  int trackNumber;
  int hasBPM;
  int bpm;
  char *comment;
  unsigned char *artworkData;
  int artworkSize;
};

static char *ttDuplicate(const char *value) {
  if(value == nullptr) { return nullptr; }
  const size_t length = std::strlen(value);
  char *copy = static_cast<char *>(std::malloc(length + 1));
  if(copy == nullptr) { return nullptr; }
  std::memcpy(copy, value, length + 1);
  return copy;
}

static char *ttDuplicateString(const TagLib::String &value) {
  if(value.isEmpty()) { return nullptr; }
  return ttDuplicate(value.toCString(true));
}

static void ttSetError(char **error, const char *message) {
  if(error != nullptr) { *error = ttDuplicate(message); }
}

static TagLib::String ttString(const char *value) {
  return TagLib::String(value == nullptr ? "" : value, TagLib::String::UTF8);
}

static char *ttProperty(const TagLib::PropertyMap &properties, const char *key) {
  const TagLib::StringList values = properties.value(ttString(key));
  if(values.isEmpty()) { return nullptr; }
  return ttDuplicateString(values.front());
}

static int ttIntProperty(const TagLib::PropertyMap &properties, const char *key, int *value) {
  char *text = ttProperty(properties, key);
  if(text == nullptr) { return 0; }
  *value = std::atoi(text);
  std::free(text);
  return *value > 0;
}

static void ttReplaceProperty(TagLib::PropertyMap &properties, const char *key, const char *value) {
  const TagLib::String tagKey = ttString(key);
  if(value != nullptr && value[0] != '\0') {
    properties.replace(tagKey, TagLib::StringList(ttString(value)));
  }
  else {
    properties.erase(tagKey);
  }
}

static void ttReplaceIntProperty(TagLib::PropertyMap &properties, const char *key, int hasValue, int value) {
  if(hasValue) {
    const std::string text = std::to_string(value);
    ttReplaceProperty(properties, key, text.c_str());
  }
  else {
    properties.erase(ttString(key));
  }
}

static const char *ttMimeType(const unsigned char *data, int size) {
  if(data == nullptr || size < 4) { return nullptr; }
  if(size >= 3 && data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF) { return "image/jpeg"; }
  if(size >= 4 && data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47) { return "image/png"; }
  return nullptr;
}

static int ttCopyArtwork(TagLib::FileRef &fileRef, TTTagLibTags *tags) {
  TagLib::List<TagLib::VariantMap> pictures = fileRef.complexProperties(ttString("PICTURE"));
  if(pictures.isEmpty()) { return 1; }

  for(auto it = pictures.begin(); it != pictures.end(); ++it) {
    TagLib::VariantMap picture = *it;
    TagLib::Variant dataVariant = picture.value(ttString("data"));
    bool ok = false;
    TagLib::ByteVector data = dataVariant.toByteVector(&ok);
    if(!ok || data.isEmpty()) { continue; }

    tags->artworkSize = static_cast<int>(data.size());
    tags->artworkData = static_cast<unsigned char *>(std::malloc(tags->artworkSize));
    if(tags->artworkData == nullptr) { return 0; }
    std::memcpy(tags->artworkData, data.data(), tags->artworkSize);
    return 1;
  }
  return 1;
}

int TTTagLibRead(const char *path, TTTagLibTags *tags, char **error) {
  if(tags == nullptr) { ttSetError(error, "Invalid tag output."); return 0; }
  std::memset(tags, 0, sizeof(TTTagLibTags));

  TagLib::FileRef fileRef(path, true);
  if(fileRef.isNull()) { ttSetError(error, "TagLib could not open the file."); return 0; }

  TagLib::PropertyMap properties = fileRef.properties();
  tags->title = ttProperty(properties, "TITLE");
  tags->artist = ttProperty(properties, "ARTIST");
  tags->album = ttProperty(properties, "ALBUM");
  tags->albumArtist = ttProperty(properties, "ALBUMARTIST");
  tags->genre = ttProperty(properties, "GENRE");
  tags->hasYear = ttIntProperty(properties, "DATE", &tags->year);
  tags->hasTrackNumber = ttIntProperty(properties, "TRACKNUMBER", &tags->trackNumber);
  tags->hasBPM = ttIntProperty(properties, "BPM", &tags->bpm);
  tags->comment = ttProperty(properties, "COMMENT");

  if(!ttCopyArtwork(fileRef, tags)) { ttSetError(error, "Failed to copy artwork data."); return 0; }
  return 1;
}

int TTTagLibWrite(const char *path, const TTTagLibTags *tags, char **error) {
  if(tags == nullptr) { ttSetError(error, "Invalid tag input."); return 0; }
  TagLib::FileRef fileRef(path, true);
  if(fileRef.isNull()) { ttSetError(error, "TagLib could not open the file."); return 0; }

  TagLib::PropertyMap properties = fileRef.properties();
  ttReplaceProperty(properties, "TITLE", tags->title);
  ttReplaceProperty(properties, "ARTIST", tags->artist);
  ttReplaceProperty(properties, "ALBUM", tags->album);
  ttReplaceProperty(properties, "ALBUMARTIST", tags->albumArtist);
  ttReplaceProperty(properties, "GENRE", tags->genre);
  ttReplaceIntProperty(properties, "DATE", tags->hasYear, tags->year);
  ttReplaceIntProperty(properties, "TRACKNUMBER", tags->hasTrackNumber, tags->trackNumber);
  ttReplaceIntProperty(properties, "BPM", tags->hasBPM, tags->bpm);
  ttReplaceProperty(properties, "COMMENT", tags->comment);
  fileRef.setProperties(properties);

  if(!fileRef.save()) { ttSetError(error, "TagLib failed to save metadata."); return 0; }
  return 1;
}

int TTTagLibWriteArtwork(const char *path, const unsigned char *data, int size, char **error) {
  const char *mimeType = ttMimeType(data, size);
  if(mimeType == nullptr) { ttSetError(error, "Unsupported artwork format."); return 0; }

  TagLib::FileRef fileRef(path, true);
  if(fileRef.isNull()) { ttSetError(error, "TagLib could not open the file."); return 0; }

  TagLib::VariantMap picture;
  picture.insert(ttString("data"), TagLib::Variant(TagLib::ByteVector(reinterpret_cast<const char *>(data), static_cast<unsigned int>(size))));
  picture.insert(ttString("mimeType"), TagLib::Variant(ttString(mimeType)));
  picture.insert(ttString("pictureType"), TagLib::Variant(ttString("Front Cover")));
  picture.insert(ttString("description"), TagLib::Variant(ttString("")));

  TagLib::List<TagLib::VariantMap> pictures;
  pictures.append(picture);
  if(!fileRef.setComplexProperties(ttString("PICTURE"), pictures)) { ttSetError(error, "TagLib failed to update artwork."); return 0; }
  if(!fileRef.save()) { ttSetError(error, "TagLib failed to save artwork."); return 0; }
  return 1;
}

int TTTagLibRemoveArtwork(const char *path, char **error) {
  TagLib::FileRef fileRef(path, true);
  if(fileRef.isNull()) { ttSetError(error, "TagLib could not open the file."); return 0; }

  TagLib::List<TagLib::VariantMap> pictures;
  if(!fileRef.setComplexProperties(ttString("PICTURE"), pictures)) { ttSetError(error, "TagLib failed to remove artwork."); return 0; }
  if(!fileRef.save()) { ttSetError(error, "TagLib failed to save artwork removal."); return 0; }
  return 1;
}

void TTTagLibFreeTags(TTTagLibTags *tags) {
  if(tags == nullptr) { return; }
  std::free(tags->title);
  std::free(tags->artist);
  std::free(tags->album);
  std::free(tags->albumArtist);
  std::free(tags->genre);
  std::free(tags->comment);
  std::free(tags->artworkData);
  std::memset(tags, 0, sizeof(TTTagLibTags));
}

void TTTagLibFreeString(char *string) {
  std::free(string);
}

}
