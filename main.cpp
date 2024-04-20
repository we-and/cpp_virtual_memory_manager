#include <iostream>
#include <vector>
#include <unordered_map>
#include <list>

#define PAGE_SIZE 1024  // Define the size of a page/frame

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
        return -1;  // No free frame
    }

    void replacePage(int oldPage, int newPage) {
        // Remove the old page from the frame it was occupying
        int frameNumber = pageTable[oldPage].frameNumber;
        frames[frameNumber] = newPage;
        pageTable[oldPage].valid = false;

        // Place the new page into the same frame
        pageTable[newPage] = {frameNumber, true};
        lruQueue.push_back(newPage);  // Add new page to the back of LRU queue
    }

public:
    VirtualMemoryManager(int numFrames) {
        frames.resize(numFrames, -1);  // Initialize all frames to be empty
    }

    int translateAddress(int virtualAddress) {
        int pageNumber = virtualAddress / PAGE_SIZE;
        int offset = virtualAddress % PAGE_SIZE;

        if (pageTable.count(pageNumber) && pageTable[pageNumber].valid) {
            // Update LRU queue: move accessed page to the back
            lruQueue.remove(pageNumber);
            lruQueue.push_back(pageNumber);
            int frameNumber = pageTable[pageNumber].frameNumber;
            return frameNumber * PAGE_SIZE + offset;
        } else {
            handlePageFault(pageNumber);
            return -1;  // Address translation failed due to page fault
        }
    }

    void handlePageFault(int pageNumber) {
        int freeFrame = findFreeFrame();
        if (freeFrame != -1) {
            // Load the new page into a free frame
            frames[freeFrame] = pageNumber;
            pageTable[pageNumber] = {freeFrame, true};
            lruQueue.push_back(pageNumber);  // Track page usage
        } else {
            // No free frame available, perform LRU replacement
            int lruPage = lruQueue.front();  // Get the least recently used page
            lruQueue.pop_front();
            replacePage(lruPage, pageNumber);
        }
        std::cout << "Page fault for page number: " << pageNumber << std::endl;
    }
};

int main() {
    VirtualMemoryManager vmm(100);  // example with 100 frames
    int physicalAddress = vmm.translateAddress(1234);
    std::cout << "Physical Address: " << physicalAddress << std::endl;
    return 0;
}