# CS2 KVM DMA

### Main Feature

- RCS
- Web-Radar
- Aimbot

```
  
TODO:
 

```

### Prerequirements

- `OpenCV` 
	- Build
	```bash
		cd opencv
		mkdir build
		cd build
		cmake -D CMAKE_BUILD_TYPE=Release -D OPENCV_GENERATE_PKGCONFIG=YES ..
		make -j16
		sudo make install
	```

	- Trouble Shooting: Cannot find LibOpenCV
	```bash
		sudo nano /etc/ld.so.conf.d/opencv.conf
		input `/usr/local/lib/`
		sudo ldconfig
	```
-  train a `.onnx` model, check [Training AI Part](https://github.com/atombottle/cs2_kvm_dma?tab=readme-ov-file#training-ai-only-tested-on-windows)
-  NPM 
	- `sudo pacman -S npm`
	- Trouble Shooting: Update gcc if it says CPP version is too low
	- Trouble Shooting: Download libs the program cannot locate(`ldd /bin/node`) if it says cannot find `libicui18n.so.75` or other `libicu` libraries(.so).
-  QMP configured on ur VM
	- follow instructions in [QeMouse](https://www.unknowncheats.me/forum/anti-cheat-bypass/491109-qemouse-injecting-mouse-events-qemu-qmp.html)
	- If VirtManager deleted it for you: 
	```c
		/*  !DO IN SAME TIME!
        1.  <domain xmlns:qemu="http://libvirt.org/schemas/domain/qemu/1.0"
        2.  <qemu:commandline>
                <qemu:arg value="-qmp"/>
                <qemu:arg value="tcp:127.0.0.1:6448,server,nowait"/>
            </qemu:commandline>
    	*/
	```

### Training AI (Only tested on Windows)

- Better targeting the blue ball twice which makes AI looks like human.
- `cd ./ai`
- run `collect_mouse_data.py`
- copy some test csv file
- run `train.py` 

### Dump .tri file from cs2 map (Only tested on Windows) (Not needed yet but suggest)
- Download Source2Viewer
- Open .vpk file of map (e.g. de_mirage.vpk)
- Export the `.vphys_c` file to `.vphys`
- Use https://github.com/atombottle/cs2-map-parser to parse it

### Complie

```bash
git clone https://github.com/atombottle/cs2_kvm_dma.git
cd cs2_kvm_dma
sudo make or sudo make -j4
```

### Configure IP for Web-Radar

- ./radar/react/App.jsx (USE_LOCALHOST) bool
- ./radar/react/App.jsx (PUBLIC_IP) string
- ./cs_dma.cpp seach`ws://` and change IP
- Port of webserver to connect is in ./radar/react/App.jsx & ./cs_dma.cpp
- Port of webpage ./react/vite.config.js
- Basically, how it work is cheat send data to web_server and react recieve the data from web_server\
  One example of how to set the network is set the ip in `cs_dma.cpp` to `127.0.0.1`, and run web_server on linux
  Then use another PC or linux to run react project\ Eventually, you can access radar on iPad&Phone&Laptop

### Sharing Radar
- TCP: port of web page (default: 8080)
- UDP: port of ws server (default: 22006)

### install dependencies(Web-Radar)

```bash
cd radar/web_server && npm install
cd radar/react && npm install
```
run `npm audit fix` if needed.

### Run

- Requirements: 
	- Put `mouse.onnx` in ./build
	- Put `.tri` files in ./build (Not needed yet but suggest)
- radar
```bash
	cd radar/web_server && node app.js
	cd radar/react && npm run dev
```

- cheat (start web_server first if you want to use web_radar)
```bash
cd build
sudo ./cs_dma
```

### Trouble Shooting: Complie libs
if you want to complie libs yourself, you might found that MemWrite is not working, you have to modify libmicroVMI:
in `./src/driver/memflow.rs`, add under `read_physical()`
```rust
    fn write_physical(&self, paddr: u64, buf: &[u8]) -> Result<(), Box<dyn Error>> {
        Ok(self.connector.borrow_mut().phys_write(PhysicalAddress::from(paddr), buf)?)
    }
```

### Credits

- Visibitlty Check: 
	- KV3 Parser https://github.com/joepriit/cpp-kv3-parser
	- https://www.unknowncheats.me/forum/counter-strike-2-a/591548-external-visibility-check-3.html
- AI: https://github.com/suixin1424/mouse_control/
- WebRadar: https://github.com/clauadv/cs2_webradar & [TKazer/CS2_External](https://github.com/TKazer/CS2_External)
- Memory / Input( Keyboard: DMA Library; Mouse: QEMU Mouse; ):
	- DMA Library: https://github.com/Metick/DMALibrary
	- QEMU Mouse: https://www.unknowncheats.me/forum/counter-strike-2-a/591548-external-visibility-check-3.html
	- LibMicroVMI: https://github.com/Wenzel/libmicrovmi
	- LeechCore Plugin[libMicroVMI]: https://github.com/ufrisk/LeechCore-plugins
	- Memflow [Connector]: https://github.com/memflow/memflow-qemu
	- Memprocfs: https://github.com/ufrisk/MemProcFS
- Json: https://github.com/nlohmann/json
