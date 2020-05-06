# OS project1 Report
###### tags: `OS`
## B07902041 邱榆洋
## 1. 設計
- 採用雙核的設定，利用 `sched_setaffinity` 來將 schedular 和 child process 分開放進不同的 CPU，也就是一個 CPU 專門跑 schedular，另一個專門在負責跑 child process。

- 利用 `sched_setscheduler` 中的參數 **SCHED_OTHER** 和 *SCHED_IDLE* 來實作每個policy，因為 **SCHED_IDLE** 會讓該 process 的 priority 變得最低，因此可以視為 "Block" 那個 process，此時再把要執行的另一個 process 用 **SCHED_OTHER** 調高 priority 即可。

- fork child process的時候，child 跑一個計時器，parent也跑一個計時器，這樣才不會出現兩個 process 有人偷跑導致開始時間跟結束時間錯誤的問題。

- FIFO
    - 就是看誰的 ready time 越小就先做，照著 ready_queue 觀地做排序，為 non_preemptive的 case。

- SJF
    - 檢查當前的 ready queue 中，誰的執行時間最小就先把誰抓出來執行，並且 non-preemptive.

- PSJF
    - 基本上跟 SJF 一樣的做法，然後因為我的程式每一輪都會把 `P[i].t_exec--` 也就是該 process 執行時間 -= 1，因此只要在每次挑選下一個 process 時多加上一個條件，檢查兩個 process 的執行時間誰比較小即可。

- RR (已修正)
    - RR 的話我是使用 queue 的方式，當 ready_time 到了時，就把process 排進 ready queue 中，如果需要 context switch，就把當前的 process 丟到 ready queue 的尾巴，然後，如果 process 結束了，就把 process 從 ready queue 中丟掉。

### system call
- mySysTime: 編號 333
    用 `getnstimeofday` 實作，但是由於這個函式中的指標會動到 user level的資料，所以必須要參數後面加上 `__user` 標注是 user level 的資料。
- myPrintk: 編號 334
    用 printk 直接輸出到 kernel
## 2. 核心版本
### Kernel: Linux ubuntu 4.14.25

## 3. 比較實際結果與理論結果

根據 output 中的數據結果，可以發現整體而言理論的時間較實際的時間來的快一些，但每個 child_process 跑的時間跟理論值沒有差得太多（僅慢了一點點），而這個差異我認為來自於以下三點：
1. scheduler 和 child process 的時間其實並不同步，而是各自跑一個 unit() 時間，然而 scedular 的程式還有許多額外的消耗，例如：選擇下一個是哪一個process來run的時間、調整 priority的時間、執行各種函式的時間。
2. 我的電腦主機本身也會有 context switch的時候，也就代表虛擬機有可能會暫時分配不到 CPU，但是計時是看當下的秒數，因此會產生誤差。
3. 每個 child process 被fork出來後，並不是馬上就被分配，會有時間差，且在死掉的時候，也不一定會馬上被 scheduler wait掉，因此一定會有誤差。