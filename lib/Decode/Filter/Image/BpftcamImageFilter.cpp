#include "BpftcamImageFilter.h"

#include "BpftHelper.h"

#include "Decode/Random/AmPrngRandom.h"
#include "Decode/Random/TrashCartPrngRandom.h"
#include "Decode/Stream/BaseFilteringReadStream.h"
#include "Shit.h"

namespace PddBy
{

//namespace Magic
//{
//
//std::string const FileHeader = "BPFTCAM";
//
//} // namespace Magic

namespace Detail
{

template<typename RandomT>
class RandomXoringReadStream : public BaseFilteringReadStream
{
    typedef std::auto_ptr<RandomT> RandomPtr;

public:
    RandomXoringReadStream(IReadStreamPtr stream, RandomPtr random) :
        BaseFilteringReadStream(stream),
        m_random(random)
    {
        //
    }

protected:
    virtual void ApplyFilter(uint8_t* buffer, std::size_t size)
    {
        for (std::size_t i = 0; i < size; i++)
        {
            buffer[i] ^= m_random->GetNext();
        }
    }

private:
    RandomPtr m_random;
};

} // namespace Detail

BpftcamImageFilter::BpftcamImageFilter(std::string const& imageName, uint16_t magicNumber, CipherType cipherType) :
    m_imageName(imageName),
    m_magicNumber(magicNumber),
    m_cipherType(cipherType)
{
    //
}

BpftcamImageFilter::~BpftcamImageFilter()
{
    //
}

IReadStreamPtr BpftcamImageFilter::Apply(IReadStreamPtr stream)
{
//    Buffer tempBuffer;
//    stream->Read(tempBuffer, Magic::FileHeader.size());
//
//    if (std::string(reinterpret_cast<char const*>(&tempBuffer[0]), tempBuffer.size()) != Magic::FileHeader)
//    {
//        throw Shit("Image is not a valid BPFTCAM image");
//    }

    switch (m_cipherType)
    {
    case TrashCartPrng:
        {
            std::auto_ptr<TrashCartPrngRandom> random(new TrashCartPrngRandom(BpftHelper::ImageNameToRandSeed(m_imageName,
                m_magicNumber)));
            return IReadStreamPtr(new Detail::RandomXoringReadStream<TrashCartPrngRandom>(stream, random));
        }

    case AmPrng:
        {
            Buffer magicKey(64);
            Buffer magicIv(64);
            for (std::size_t i = 0; i < 64; i++)
            {
                magicKey[i] = i & 5;
                magicIv[i] = i & 2;
            }

            std::auto_ptr<AmPrngRandom> random(new AmPrngRandom(magicKey, magicIv,
                BpftHelper::ImageNameToRandSeed(m_imageName, m_magicNumber) % 0x300));
            return IReadStreamPtr(new Detail::RandomXoringReadStream<AmPrngRandom>(stream, random));
        }
    }

    throw Shit("Unsupported cipher type");
}

} // namespace PddBy
