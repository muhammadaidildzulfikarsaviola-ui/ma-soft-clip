#pragma once

#include <juce_core/juce_core.h>
#include <vector>

template <typename T>
class AudioFifo
{
public:
    explicit AudioFifo(int size = 4096)
        : abstractFifo(size), buffer(static_cast<size_t>(size), T(0))
    {}

    void setSize(int size)
    {
        abstractFifo.setTotalSize(size);
        buffer.assign(static_cast<size_t>(size), T(0));
    }

    int getCapacity() const noexcept
    {
        return abstractFifo.getTotalSize();
    }

    int getNumReady() const noexcept
    {
        return abstractFifo.getNumReady();
    }

    // Audio Thread: Menulis array sampel tanpa alokasi memori
    void write(const T* data, int numSamples) noexcept
    {
        int start1, size1, start2, size2;
        abstractFifo.prepareToWrite(numSamples, start1, size1, start2, size2);

        if (size1 > 0)
            std::copy(data, data + size1, buffer.begin() + start1);
        if (size2 > 0)
            std::copy(data + size1, data + size1 + size2, buffer.begin() + start2);

        abstractFifo.finishedWrite(size1 + size2);
    }

    // GUI Thread: Membaca sampel yang tersedia
    int read(T* destination, int numSamples) noexcept
    {
        int start1, size1, start2, size2;
        abstractFifo.prepareToRead(numSamples, start1, size1, start2, size2);

        if (size1 > 0)
            std::copy(buffer.begin() + start1, buffer.begin() + start1 + size1, destination);
        if (size2 > 0)
            std::copy(buffer.begin() + start2, buffer.begin() + start2 + size2, destination + size1);

        abstractFifo.finishedRead(size1 + size2);
        return size1 + size2;
    }

private:
    juce::AbstractFifo abstractFifo;
    std::vector<T> buffer;
};