#pragma once

#include "Image.h"
#include "CopyOnWrite.h"

namespace IRL
{
    class GaussianPyramid
    {
        IMPLEMENT_COPY_ON_WRITE(GaussianPyramid);
    public:
        GaussianPyramid(const Image& image, int levels)
        {
            _ptr = new Private(image, levels);
        }

        int GetLevelsCount() const
        {
            ASSERT(IsValid());
            return _ptr->Levels.size();
        }

        const Image& GetLevel(int level) const
        {
            ASSERT(IsValid());
            ASSERT(level < GetLevelsCount());
            return _ptr->Levels[level];
        }

        Image& GetLevel(int level)
        {
            ASSERT(IsValid());
            ASSERT(level < GetLevelsCount());
            MakePrivate();
            return _ptr->Levels[level];
        }

        void Save(const std::string& filePath)
        {
            ASSERT(IsValid());
            _ptr->Save(filePath);
        }

    private:
        class Private :
            public RefCounted<Private>
        {
            Private(const Private& copyFrom) : Levels(copyFrom.Levels) {}
        public:
            Private(const Image& image, int levels);
            Private* Clone() const { return new Private(*this); }
            static void Delete(Private* obj) { delete obj; }

            void Save(const std::string& filePath);

        public:
            std::vector<Image> Levels;
        };
    };
}