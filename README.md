# CSGO KVM DMA

### Main Feature
1. Triggerbot with random press/release time

```
	*TODO list:
	add method to detect key event in VM.
	add No recoil.
	add wallhack(maybe radar , glow ..... no idea about it)*
```
### How to use
	Run a VM with GPU passthrough.
	in linux , run "build.sh" to build this. Don't forget to install rustc,gcc,make..
	please correct me if wrong.

### about codes
	Used Hazedumper offsets
 	Used Memflow LIB
  
	Just added one function getModuleBase() to memory part.
	Other codes in memory R/W part are pasted from [apex kvm dma pub](https://github.com/MisterY52/apex_dma_kvm_pub "apex kvm dma pub")
	There's some error when use“player_index + offset”, so some offset are converted to INT format. I choosed the easiest way :)

### about update
	This repo was done in July.
	But I'm in a boarding school now. Unluckily, I didn't know there's lots of people bring their PC with monitor here until last Thursday a friend told me that. So I brought my laptop when AUG 2th. but it can't work well with playing games in VM although I enable KVM/GPU passthrough.
	I'll try to bring my PC here when Christmas vacation.
	At that time, I'll try to coding new features to this repository.





