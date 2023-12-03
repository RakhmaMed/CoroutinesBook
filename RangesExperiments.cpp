#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iterator> // for std::input_iterator_tag
#include <ranges>   // for std::views::transform
#include <memory>


#include <Packet.h>
#include <PcapFileDevice.h>
#include <TcpLayer.h>

import Generator;

void analyzePacket(pcpp::RawPacket& rawPacket, std::ofstream& outputFile)
{
    pcpp::Packet parsedPacket(&rawPacket);

    if (parsedPacket.isPacketOfType(pcpp::TCP))
    {
        pcpp::TcpLayer* tcpLayer = parsedPacket.getLayerOfType<pcpp::TcpLayer>();

        if (tcpLayer->getTcpHeader()->portDst == htons(554) || tcpLayer->getTcpHeader()->portSrc == htons(554))
        {
            uint8_t* data = tcpLayer->getLayerPayload();
            size_t dataLen = tcpLayer->getLayerPayloadSize();

            outputFile.write(reinterpret_cast<const char*>(data), dataLen);
        }
    }
}

auto deleter = [](auto* obj)
    {
        std::cout << "delte " << obj << '\n';
        delete obj;
    };

class PacketIterable {
public:
    explicit PacketIterable(const std::string& inputPath) noexcept
        : inputPath_(inputPath)
        , reader_(pcpp::IFileReaderDevice::getReader(inputPath))
    {}

    ~PacketIterable() noexcept {
        if (reader_->isOpened()) {
            reader_->close();
        }
        delete reader_;
    }


    bool open() {
        return reader_->open();
    }

    class PacketIterator {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = pcpp::Packet;
        using iterator_category = std::input_iterator_tag;

        PacketIterator() : m_reader(nullptr) {};
        PacketIterator(pcpp::IFileReaderDevice* reader) noexcept
            : m_reader(reader)
        {
            if (reader)
                nextPacket();
        }

        bool operator==(const PacketIterator& other) const noexcept {
            return m_reader == other.m_reader;
        }

        bool operator!=(const PacketIterator& other) const noexcept {
            return m_reader != other.m_reader;
        }

        bool nextPacket() {
            m_packet.~Packet();
            return m_reader->getNextPacket(m_rawPacket);
        }

        PacketIterator& operator++() noexcept {
            if (!nextPacket()) {
                m_reader = nullptr;  // mark as end
            }
            return *this;
        }

        void operator++(int) noexcept {
            this->operator++();
        }

        const value_type& operator*() const {
            m_packet = std::move(pcpp::Packet(&m_rawPacket));
            return m_packet;
        }

    private:
        pcpp::IFileReaderDevice* m_reader;
        pcpp::RawPacket mutable m_rawPacket;
        pcpp::Packet mutable m_packet;
    };

    using value_type = pcpp::Packet;
    using iterator = PacketIterator;

    iterator begin() { return iterator(reader_); }
    iterator end() { return iterator(nullptr); }


    std::string inputPath_;
    pcpp::IFileReaderDevice* reader_;
};



int main(int argc, char* argv[])
{
    std::string inputPath = R"(C:\Users\irahm\Downloads\export.pcapng)";
    std::string outputPath = R"(C:\Users\irahm\Documents\output.txt)";

    std::ofstream outputFile(outputPath);

    if (!outputFile.is_open())
    {
        std::cerr << "Error opening the output file!" << std::endl;
        return 1;
    }

    PacketIterable packets(inputPath);

    if (!packets.open()) {
        std::cerr << "Error opening the pcap file!" << std::endl;
        return 1;
    }

    using namespace std::views;
    auto views = packets
        | filter([](auto&& packet) { std::cout << "second\n";  return packet.isPacketOfType(pcpp::TCP); })
        | transform([](auto&& packet) { std::cout << "third\n";  return packet.getLayerOfType<pcpp::TcpLayer>(); })
        | filter([](auto tcpLayer) {
        std::cout << "fourth\n";
        return tcpLayer->getTcpHeader()->portDst == htons(554) || tcpLayer->getTcpHeader()->portSrc == htons(554);
            })
        | transform([](auto tcpLayer) {
                std::cout << "fifth\n";
                return std::string_view{ reinterpret_cast<const char*>(tcpLayer->getLayerPayload()), tcpLayer->getLayerPayloadSize() };
            });

            for (auto packet : views) {
                std::cout << packet;
                //outputFile.write(packet.data(), packet.size());
            }

            return 0;
}
