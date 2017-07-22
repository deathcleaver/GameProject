#include "GameFile.hpp"

#include <AssetLib/AssetLib.hpp>

GameFile::~GameFile() {

	EntryMap::const_iterator it = loadedEntries.begin();
	EntryMap::const_iterator eit = loadedEntries.end();

	for (it; it != eit; it++) {
		delete[] it->second.data;
	}

}

void GameFile::init() {
	inFile.rdbuf()->pubsetbuf(nullptr, 0);
}

void GameFile::load(const char* name) {

	fileName = name;

	uint32_t dataSize = 0;
	void* data = AssetLib::fileToMemory(name, dataSize);

	if (data) {
		Version* v = (Version*)data;

		printf("Loading file with version %d\n", v->version);

		if (v->version == 1) {
			GameFileHeader_V1* gameFileV1 = (GameFileHeader_V1*)data;

			nrEntries = gameFileV1->nrEntries;
			EntryOffset* offset = (EntryOffset*)&((char*)data)[sizeof(GameFileHeader_V1)];
			for (uint32_t i = 0; i < nrEntries; i++) {
				offsetTable[offset[i].formID] = offset[i].offset;
			}

		} else if (v->version == 2) {
			GameFileHeader_V2* gameFileV1 = (GameFileHeader_V2*)data;
			header = *gameFileV1;
			nrEntries = gameFileV1->nrEntries;
			EntryOffset* offset = (EntryOffset*)&((char*)data)[sizeof(GameFileHeader_V2)];
			for (uint32_t i = 0; i < nrEntries; i++) {
				offsetTable[offset[i].formID] = offset[i].offset;
			}
		}

		delete[] data;

		inFile.open(name, std::ios::binary);

		//Entry e{};
		//e.formID = 2;
		//memcpy(e.tag, "TAG1", 4);
		//e.entrySize = sizeof(EntrySave) + 100;
		//e.data = new char[100];
		//
		//memset(e.data, 0, 100);
		//memcpy(e.data, "Third entry with a different tag", 33);
		//
		//loadedEntries[e.formID] = e;
		//
		//nrEntries++;

	} else {
		// failed to read file
	}

}

Entry* GameFile::loadEntry(uint32_t formId) {

	Entry* entry = nullptr;

	// already loaded
	if (loadedEntries.count(formId)) {
		entry = &loadedEntries[formId];
	} else if (offsetTable.count(formId)) {
		inFile.seekg(0);
		uint32_t offset = offsetTable[formId];
		inFile.seekg(offset, std::ios::beg);

		Entry e{};
		inFile.read((char*)&e, sizeof(EntrySave));

		if (formId == e.formID) {

			uint32_t dataSize = e.entrySize - sizeof(EntrySave);
			if (dataSize) {
				e.data = new char[dataSize];
				inFile.read((char*)e.data, dataSize);
			} else {
				e.data = nullptr;
			}

			loadedEntries[formId] = e;
			entry = &loadedEntries[formId];
			Tag t;
			memcpy(&t, e.tag, sizeof(Tag));
			tagMap[t].push_back(formId);
		} else {
			printf("Faulty entry offset for entry! id %d was supposed to be %d \n", e.formID, formId);
		}
	} else {
		printf("Error : form not found\n");
	}

	return entry;
}

uint32_t GameFile::getNextFormID() {

	uint32_t id = 0;
	for (uint32_t i = 0; i < nrEntries; i++) {
		if (offsetTable.count(i) == 0) {
			id = i;
			break;
		}
	}

	if (id == 0) {
		id = nrEntries;
	}

	return id;
}

void GameFile::createNewEntry(uint32_t formID, const char* tag) {

	Entry e{};
	e.formID = formID;
	memcpy(e.tag, tag, 4);
	e.entrySize = sizeof(EntrySave);
	e.data = nullptr;

	loadedEntries[e.formID] = e;
	offsetTable[e.formID] = 0;

	nrEntries++;
}

GameFileHeader_V2 GameFile::getHeaderDataV2() const {
	return header;
}

void GameFile::loadAllMissingEntries() {

	OffsetTable::const_iterator it = offsetTable.begin();
	OffsetTable::const_iterator eit = offsetTable.end();

	for (it; it != eit; it++) {
		if (loadedEntries.count(it->first) == 0) {
			loadEntry(it->first);
		}
	}
}

void GameFile::updateEntry(Entry* entry) {
	loadedEntries[entry->formID] = *entry;
}