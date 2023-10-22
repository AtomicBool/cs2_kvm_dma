# CSGO KVM DMA

### Main Feature
1. Triggerbot with random press/release time

```
	*TODO list: start in 2024/1？
         Working on the cheat for ApexLegends
	Try to fix the issue with the format of Offsets in this project

```
### How to use
	Run a VM with GPU passthrough.
	in Linux, run "build.sh" to build this. Don't forget to install rustc,gcc,make..
	please correct me if wrong.

### about codes
	Used Hazedumper's offsets
 	Used Memflow LIB
  
	Just added one function getModuleBase() to the memory part.
	Other codes in memory R/W part are pasted from [apex kvm dma pub](https://github.com/MisterY52/apex_dma_kvm_pub "apex kvm dma pub")
	Some errors happened when using “player_index + offset”, so some offsets are converted to INT format. I chose the easiest way :)

### about update
	Waiting for my PC with GPU passthrough to make a new one for CS2





