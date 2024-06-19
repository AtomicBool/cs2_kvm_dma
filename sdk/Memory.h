#pragma once
#include "pch.h"
#include "InputManager.h"
#include "ray_trace.h"

struct CurrentProcessInformation
{
	int PID = 0;
	size_t base_address = 0;
	size_t base_size = 0;
	std::string process_name = "";
};

class Memory
{
private:
	static inline CurrentProcessInformation current_process { };

	static inline BOOLEAN DMA_INITIALIZED = FALSE;
	static inline BOOLEAN PROCESS_INITIALIZED = FALSE;
	/**
	*Dumps the systems Current physical memory pages
	*To a file so we can use it in our DMA (:
	*This file it created to %temp% folder
	*@return true if successful, false if not.
	*/
	bool DumpMemoryMap(bool debug = false);

	//shared pointer
	std::shared_ptr<c_keys> key;

	/*this->registry_ptr = std::make_shared<c_registry>(*this);
	this->key_ptr = std::make_shared<c_keys>(*this);*/

	VMMDLL_SCATTER_HANDLE scatter_handle;
	std::mutex m;

public:
	/**
	 * brief Constructor takes a wide string of the process.
	 * Expects that all the libraries are in the root dir
	 */
	Memory();
	~Memory();

	/**
	* @brief Gets the key object
	* @return key class
	*/
	c_keys* GetKeyboard() { return key.get(); }

	/**
	* brief Initializes the DMA
	* This is required before any DMA operations can be done.
	* @param process_name the name of the process
	* @param memMap if true, will dump the memory map to a file	& make the DMA use it.
	* @return true if successful, false if not.
	*/
	bool Init(std::string process_name, bool memMap = true, bool debug = false);

	/*This part here is things related to the process information such as Base daddy, Size ect.*/

	/**
	* brief Gets the process id of the process
	* @param process_name the name of the process
	* @return the process id of the process
	*/
	DWORD GetPidFromName(std::string process_name);

	/**
	* brief Gets all the processes id(s) of the process
	* @param process_name the name of the process
	* @returns all the processes id(s) of the process
	*/
	std::vector<int> GetPidListFromName(std::string process_name);

	/**
	* \brief Gets the module list of the process
	* \param process_name the name of the process 
	* \return all the module names of the process 
	*/
	std::vector<std::string> GetModuleList(std::string process_name);

	/**
	* \brief Gets the process information
	* \return the process information
	*/
	VMMDLL_PROCESS_INFORMATION GetProcessInformation();

	/**
	* brief Gets the base address of the process
	* @param module_name the name of the module
	* @return the base address of the process
	*/
	size_t GetBaseDaddy(std::string module_name);

	/**
	* brief Gets the base size of the process
	* @param module_name the name of the module
	* @return the base size of the process
	*/
	size_t GetBaseSize(std::string module_name);

	/**
	* brief Gets the export table address of the process
	* @param import the name of the export
	* @param process the name of the process
	* @param module the name of the module that you wanna find the export in
	* @return the export table address of the export
	*/
	uintptr_t GetExportTableAddress(std::string import, std::string process, std::string module);

	/**
	* brief Gets the import table address of the process
	* @param import the name of the import
	* @param process the name of the process
	* @param module the name of the module that you wanna find the import in
	* @return the import table address of the import
	*/
	uintptr_t GetImportTableAddress(std::string import, std::string process, std::string module);

	/**
	 * \brief This fixes the CR3 fuckery that EAC does.
	 * It fixes it by iterating over all DTB's that exist within your system and looks for specific ones
	 * that nolonger have a PID assigned to them, aka their pid is 0
	 * it then puts it in a vector to later try each possible DTB to find the DTB of the process.
	 * NOTE: Using FixCR3 requires you to have symsrv.dll, dbghelp.dll and info.db
	 */
	bool FixCr3();

	/*This part is where all memory operations are done, such as read, write.*/

	/**
	 * \brief Scans the process for the signature.
	 * \param signature the signature example "48 ? ? ?"
	 * \param range_start Region to start scan from 
	 * \param range_end Region up to where it should scan
	 * \param PID (OPTIONAL) where to read to?
	 * \return address of signature
	 */
	uint64_t FindSignature(const char* signature, uint64_t range_start, uint64_t range_end, int PID = 0);

	template<typename T>
	T Read(uint64_t address);

	std::string readString(uint64_t addr);
	std::string readString(uint64_t addr, int len);

	template<typename T>
	T ReadPID(uint64_t address, int pid);

	template<typename T>
	void ReadArray(uint64_t address, T out[], size_t len);

	template<typename T>
	void Write(uint64_t address, T value);

	template<typename T>
	void WriteArray(uint64_t address, T value[], size_t len);

	/**
	 * \brief Create a scatter handle, this is used for scatter read/write requests
	 * \return Scatter handle
	 */
	VMMDLL_SCATTER_HANDLE CreateScatterHandle() const;
	VMMDLL_SCATTER_HANDLE CreateScatterHandle(int pid) const;

	/**
	 * \brief Closes the scatter handle
	 * \param handle
	 */
	void CloseScatterHandle(VMMDLL_SCATTER_HANDLE handle);

	/**
	 * \brief Adds a scatter read/write request to the handle
	 * \param handle the handle
	 * \param address the address to read/write to 
	 * \param buffer the buffer to read/write to
	 * \param size the size of buffer
	 */
	void AddScatterReadRequest(VMMDLL_SCATTER_HANDLE handle, uint64_t address, void* buffer, size_t size);
	void AddScatterWriteRequest(VMMDLL_SCATTER_HANDLE handle, uint64_t address, void* buffer, size_t size);

	/**
	 * \brief Executes all prepared scatter requests, note if you created a scatter handle with a pid
	 * you'll need to specify the pid in the execute function. so we can clear the scatters from the handle.
	 * \param handle 
	 * \param pid 
	 */
	void ExecuteReadScatter(VMMDLL_SCATTER_HANDLE handle, int pid = 0);
	void ExecuteWriteScatter(VMMDLL_SCATTER_HANDLE handle, int pid = 0);

	uint64_t get_base_address();
	uint64_t get_base_address(const char* module_name);

	/*the FPGA handle*/
	VMM_HANDLE vHandle;
	uint64_t base; //only used in CR3Fix
};

template<typename T>
inline T Memory::Read(uint64_t address)
{
	std::lock_guard<std::mutex> l(m);
	T buf;
	if(scatter_handle) AddScatterReadRequest(scatter_handle, address, &buf, sizeof(T));
	ExecuteReadScatter(scatter_handle);
	return buf;
}

inline std::string Memory::readString(uint64_t addr) {
    return readString(addr, 64);
}

inline std::string Memory::readString(uint64_t addr, int len) {
    std::vector<uint8_t> string_bytes;

    for(int i = 0; i < len; i += 1){
	string_bytes.push_back(Read<uint8_t>(addr + i));
    }

    std::vector<char> str(len);
    for (int i = 0; i < len; i++) {
	if(string_bytes[i] == 0 ){
        	str.resize(i);
		break;
	}
	str[i] = string_bytes[i];
    }

    std::string res(str.begin(), str.end());

    return res;
}


template<typename T>
inline T Memory::ReadPID(uint64_t address, int pid)
{
	std::lock_guard<std::mutex> l(m);
	auto scatter_handle_ = CreateScatterHandle(pid); //not frequently, close after create
	T buf;
	if(scatter_handle_) AddScatterReadRequest(scatter_handle_, address, &buf, sizeof(T));
	ExecuteReadScatter(scatter_handle_);
	CloseScatterHandle(scatter_handle_);
	return buf;
}

template<typename T>
inline void Memory::ReadArray(uint64_t address, T out[], size_t len)
{
	std::lock_guard<std::mutex> l(m);
	if(scatter_handle) AddScatterReadRequest(scatter_handle, address, (T*)out, sizeof(T) * len);
	ExecuteReadScatter(scatter_handle);
}

template<typename T>
inline void Memory::Write(uint64_t address, T value)
{
	std::lock_guard<std::mutex> l(m);
	if(scatter_handle) AddScatterWriteRequest(scatter_handle, address, &value, sizeof(T));
	ExecuteWriteScatter(scatter_handle);
}

template<typename T>
inline void Memory::WriteArray(uint64_t address, T value[], size_t len)
{
	std::lock_guard<std::mutex> l(m);
	if(scatter_handle) AddScatterReadRequest(scatter_handle, address, (T*)value, sizeof(T) * len);
	ExecuteWriteScatter(scatter_handle);
}

inline Memory mem;
