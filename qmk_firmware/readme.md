## nucyrosp

QMK Firmware 0.29.6

quantum/split_common/transactions.c

因為不知道pmw3610 get cpi的正確操作方式，

參考了幾個zmk firmware repo的寫法，都沒有成功。

雙手都裝了pmw3610，master切換cpi正常，

slave要切換cpi需要用pointing_device_set_cpi_on_side()，但它寫完後會get_cpi檢查是否寫入成功。

剛好就這個get cpi讀取失敗後會開始無限重試拖垮整個系統，所以就直接把這段程式屏蔽掉。

```
#if 0
    uint16_t temp_cpi = !pointing_device_driver->get_cpi ? 0 : pointing_device_driver->get_cpi(); // check for NULL
#endif
   

#if 0
    if (pointing.cpi && pointing.cpi != temp_cpi && pointing_device_driver->set_cpi) {
        pointing_device_driver->set_cpi(pointing.cpi);
    }
#else
    static uint16_t tmp_cpi=0;
    if(tmp_cpi != pointing.cpi){
        tmp_cpi = pointing.cpi;
        pointing_device_driver->set_cpi(pointing.cpi);
    }

#endif
```

