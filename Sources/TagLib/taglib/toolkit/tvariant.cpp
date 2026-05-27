/***************************************************************************
    copyright            : (C) 2023 by Urs Fleisch
    email                : ufleisch@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#include "tvariant.h"

#include <variant>
#include <iomanip>

#include "tstring.h"
#include "tstringlist.h"
#include "tbytevector.h"
#include "tbytevectorlist.h"
#include "fileref.h"
#include "tpropertymap.h"

#include <cstdlib>
#include <cctype>
#include <cstring>
#include <string>

using TagLib::ByteVector;
using TagLib::ByteVectorList;
using TagLib::List;
using TagLib::Map;
using TagLib::String;
using TagLib::StringList;
using TagLib::Variant;
using TagLib::VariantList;
using TagLib::VariantMap;

namespace {

// The number and order of the template parameters must correspond to the
// enum values in Variant::Type!
using StdVariantType = std::variant<
  std::monostate,
  bool,
  int,
  unsigned int,
  long long,
  unsigned long long,
  double,
  TagLib::String,
  TagLib::StringList,
  TagLib::ByteVector,
  TagLib::ByteVectorList,
  List<TagLib::Variant>,
  Map<TagLib::String, TagLib::Variant>
>;

template<typename T>
T getVariantValue(StdVariantType *v, bool *ok)
{
  if(const auto valPtr = std::get_if<T>(v)) {
    if(ok) {
      *ok = true;
    }
    return *valPtr;
  }
  if(ok) {
    *ok = false;
  }
  return {};
}

void printVariantToStream(std::ostream &, const StdVariantType &)
{
}

} // namespace

class Variant::VariantPrivate
{
public:
  VariantPrivate() = default;
  VariantPrivate(StdVariantType v) : data(std::move(v)) {}
  StdVariantType data;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

Variant::Variant() :
  d(std::make_shared<VariantPrivate>())
{
}

Variant::Variant(int val) :
  d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(unsigned int val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(long long val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(unsigned long long val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(bool val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(double val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(const char *val) :
 d(std::make_shared<VariantPrivate>(TagLib::String(val)))
{
}

Variant::Variant(const TagLib::String &val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(const TagLib::StringList &val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(const TagLib::ByteVector &val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(const TagLib::ByteVectorList &val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(const TagLib::List<TagLib::Variant> &val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(const TagLib::Map<TagLib::String, TagLib::Variant> &val) :
 d(std::make_shared<VariantPrivate>(val))
{
}

Variant::Variant(const Variant &) = default;

////////////////////////////////////////////////////////////////////////////////

Variant::~Variant() = default;

Variant::Type Variant::type() const {
  return static_cast<Type>(d->data.index());
}

bool Variant::isEmpty() const
{
  return type() == Void;
}

template<typename T>
T Variant::value(bool *ok) const
{
  return getVariantValue<T>(&d->data, ok);
}

template bool Variant::value(bool *ok) const;
template int Variant::value(bool *ok) const;
template unsigned int Variant::value(bool *ok) const;
template long long Variant::value(bool *ok) const;
template unsigned long long Variant::value(bool *ok) const;
template double Variant::value(bool *ok) const;
template String Variant::value(bool *ok) const;
template StringList Variant::value(bool *ok) const;
template ByteVector Variant::value(bool *ok) const;
template ByteVectorList Variant::value(bool *ok) const;
template VariantList Variant::value(bool *ok) const;
template VariantMap Variant::value(bool *ok) const;

bool Variant::toBool(bool *ok) const
{
  return value<bool>(ok);
}

int Variant::toInt(bool *ok) const
{
  return value<int>(ok);
}

unsigned int Variant::toUInt(bool *ok) const
{
  return value<unsigned int>(ok);
}

long long Variant::toLongLong(bool *ok) const
{
  return value<long long>(ok);
}

unsigned long long Variant::toULongLong(bool *ok) const
{
  return value<unsigned long long>(ok);
}

double Variant::toDouble(bool *ok) const
{
  return value<double>(ok);
}

TagLib::String Variant::toString(bool *ok) const
{
  return value<TagLib::String>(ok);
}

TagLib::StringList Variant::toStringList(bool *ok) const
{
  return value<TagLib::StringList>(ok);
}

TagLib::ByteVector Variant::toByteVector(bool *ok) const
{
  return value<TagLib::ByteVector>(ok);
}

TagLib::ByteVectorList Variant::toByteVectorList(bool *ok) const
{
  return value<TagLib::ByteVectorList>(ok);
}

TagLib::List<TagLib::Variant> Variant::toList(bool *ok) const
{
  return value<TagLib::List<TagLib::Variant>>(ok);
}

TagLib::Map<TagLib::String, TagLib::Variant> Variant::toMap(bool *ok) const
{
  return value<TagLib::Map<TagLib::String, TagLib::Variant>>(ok);
}

bool Variant::operator==(const Variant &v) const
{
  return d == v.d || d->data == v.d->data;
}

bool Variant::operator!=(const Variant &v) const
{
  return !(*this == v);
}

Variant &Variant::operator=(const Variant &) = default;

////////////////////////////////////////////////////////////////////////////////
// related non-member functions
////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &s, const TagLib::Variant &v)
{
  printVariantToStream(s, v.d->data);
  return s;
}

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

static char *ttPropertyWithFallback(const TagLib::PropertyMap &properties, const char *primaryKey, const char *fallbackKey) {
  char *value = ttProperty(properties, primaryKey);
  if(value != nullptr) { return value; }
  return ttProperty(properties, fallbackKey);
}

static int ttIntProperty(const TagLib::PropertyMap &properties, const char *key, int *value) {
  char *text = ttProperty(properties, key);
  if(text == nullptr) { return 0; }
  *value = std::atoi(text);
  std::free(text);
  return *value > 0;
}

static int ttIntPropertyWithFallback(const TagLib::PropertyMap &properties, const char *primaryKey, const char *fallbackKey, int *value) {
  if(ttIntProperty(properties, primaryKey, value)) { return 1; }
  return ttIntProperty(properties, fallbackKey, value);
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

static void ttReplaceIntPropertyWithAlias(TagLib::PropertyMap &properties, const char *primaryKey, const char *aliasKey, int hasValue, int value) {
  ttReplaceIntProperty(properties, primaryKey, hasValue, value);
  ttReplaceIntProperty(properties, aliasKey, hasValue, value);
}

static bool ttHasExtension(const char *path, const char *extension) {
  if(path == nullptr || extension == nullptr) { return false; }

  const char *dot = std::strrchr(path, '.');
  if(dot == nullptr) { return false; }
  ++dot;

  while(*dot != '\0' && *extension != '\0') {
    const unsigned char lhs = static_cast<unsigned char>(*dot);
    const unsigned char rhs = static_cast<unsigned char>(*extension);
    if(std::tolower(lhs) != std::tolower(rhs)) { return false; }
    ++dot;
    ++extension;
  }
  return *dot == '\0' && *extension == '\0';
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
