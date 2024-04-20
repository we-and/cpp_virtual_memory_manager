#include <iostream>
#include <vector>
#include <unordered_map>
#include <list>
#include <stdexcept> // include for std::runtime_error

#define PAGE_SIZE 1024  // size of a page/frame

struct PageTableEntry {
    int frameNumber;
    bool valid;
};

class VirtualMemoryManager {
private:
    std::unordered_map<int, PageTableEntry> pageTable;
    std::vector<int> frames;  // Stores the page number that occupies each frame
    std::list<int> lruQueue;  // LRU queue to track page usage

    int findFreeFrame() {
        for (int i = 0; i < frames.size(); ++i) {
            if (frames[i] == -1) return i;  // Frame is free
        }
        return -1;  // no free frame
    }

    void replacePage(int oldPage, int newPage) {
        // Error check if old page does not exist in page table
        if (pageTable.find(oldPage) == pageTable.end() || !pageTable[oldPage].valid) {
            throw std::runtime_error("Attempt to replace a non-existent or invalid page.");
        }

        int frameNumber = pageTable[oldPage].frameNumber;
        frames[frameNumber] = newPage;
        pageTable[oldPage].valid = false;

        pageTable[newPage] = {frameNumber, true};
        lruQueue.push_back(newPage);
    }

public:
    VirtualMemoryManager(int numFrames) {
        frames.resize(numFrames, -1);
    }

    int translateAddress(int virtualAddress) {
        int pageNumber = virtualAddress / PAGE_SIZE;
        int offset = virtualAddress % PAGE_SIZE;

        try {
            if (pageTable.count(pageNumber) && pageTable[pageNumber].valid) {
                lruQueue.remove(pageNumber);
                lruQueue.push_back(pageNumber);
                int frameNumber = pageTable[pageNumber].frameNumber;
                return frameNumber * PAGE_SIZE + offset;
            } else {
                handlePageFault(pageNumber);
                return translateAddress(virtualAddress); // Retry translation after handling page fault
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return -1;  // Return error code
        }
    }

    void handlePageFault(int pageNumber) {
        int freeFrame = findFreeFrame();
        if (freeFrame != -1) {
            frames[freeFrame] = pageNumber;
            pageTable[pageNumber] = {freeFrame, true};
            lruQueue.push_back(pageNumber);
        } else {
            if (!lruQueue.empty()) {
                int lruPage = lruQueue.front();
                lruQueue.pop_front();
                replacePage(lruPage, pageNumber);
            } else {
                throw std::runtime_error("No pages in LRU queue to replace.");
            }
        }
        std::cout << "Page fault handled for page number: " << pageNumber << std::endl;
    }
};

int main() {
    VirtualMemoryManager vmm(100);
    try {
        int physicalAddress = vmm.translateAddress(1234);
        std::cout << "Physical Address: " << physicalAddress << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }
    return 0;
}