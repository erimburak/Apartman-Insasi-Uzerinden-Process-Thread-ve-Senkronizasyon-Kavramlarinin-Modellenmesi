# 🏗️ Çok Katlı Bir Apartmanın İnşası Üzerinden Process, Thread ve Senkronizasyon Kavramlarının Modellenmesi

**İşletim Sistemleri Dersi Proje Ödevi**

---

## Kısa Açıklama

Bu projede, çok katlı bir apartman inşaatı simülasyonu üzerinden işletim sistemi kavramları (proses, thread, mutex, semafor, paylaşımlı bellek) C diliyle modellenmiştir. Her kat bağımsız bir proses olarak, her daire ise thread olarak çalışır. Sınırlı kaynaklar (vinç, asansör, elektrik/su tesisatı) mutex ve semaforlarla senkronize edilir. Rastgele olaylar ile simülasyon gerçek hayata yakınlaştırılmıştır. Kullanıcıdan tek beklenti, programı başlatıp çıktıyı gözlemlemesidir.

---

## 📁 Klasör/Dosya Yapısı

isletimSistemiProje/
├── isletimSistemiProje.cbp # Code::Blocks proje dosyası
├── isletimSistemiProje.layout # IDE yerleşim dosyası
├── main.c # C kaynak dosyası (ana kod)
├── simulasyon # Derlenmiş yürütülebilir dosya (Linux)
├── bin/
│ └── Debug/
│ └── isletimSistemiProje # Windows exe
├── obj/
│ └── Debug/
│ └── main.o # Derlenmiş obje dosyası


---

## ⚙️ Kurulum ve Çalıştırma

> **Not:** Proje Linux tabanlı sistemde sorunsuz çalışır.  
> Windows’ta Code::Blocks veya başka bir C IDE’si ile açıp derleyebilirsiniz.

### Terminalden Derleme ve Çalıştırma (Linux)

## ⚙️ Kurulum ve Çalıştırma

1. **Gerekli Kurulumlar:**  
   Proje Linux ortamında derlenip test edilmiştir. `gcc` ve `pthread` kütüphanesi gereklidir.

2. **Derleme:**
    ```bash
    gcc -o simulasyon main.c -lpthread
    ```
    veya mevcut binary dosyası ile direkt çalıştırabilirsiniz.

3. **Çalıştırma:**
    ```bash
    ./simulasyon
    ```

---

## 🧩 Projenin Ana Mantığı ve Temel Bileşenler

- **Proses ve Thread Kullanımı:**  
  Her kat için yeni bir process (`fork()`), her daire için thread (`pthread_create`) açılır.

    ```c
    // Katlar için process oluşturma (örnek)
    for (int i = 1; i <= TOTAL_FLOORS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            build_floor(i); // Alt kat bitmeden başlamaz!
            exit(0);
        }
    }

    // Daireler için thread oluşturma (örnek)
    pthread_t threads[FLATS_PER_FLOOR];
    for (int j = 0; j < FLATS_PER_FLOOR; j++) {
        pthread_create(&threads[j], NULL, build_flat, (void*)&flat_args[j]);
    }
    ```

- **Paylaşımlı Bellek Kullanımı:**  
  Katların tamamlanma durumu mmap ile tüm proseslere açık bir alanda tutulur.  
    ```c
    kat_durumu = mmap(NULL, sizeof(int) * (TOTAL_FLOORS + 1),
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    ```

- **Kaynak Senkronizasyonu:**  
  Vinç, asansör, elektrik/su tesisatı gibi kaynaklar mutex veya semafor ile yönetilir.
    ```c
    pthread_mutex_lock(&crane_mutex);
    // Vinç kullanımı...
    pthread_mutex_unlock(&crane_mutex);

    sem_wait(&elevator_sem);
    // Asansörle taşıma...
    sem_post(&elevator_sem);
    ```

- **Rastgele Olaylar:**  
  Simülasyona gerçekçilik katmak için rastgele olaylar tetiklenir.
    ```c
    int chance = rand() % 10;
    if (chance == 0) {
        printf("Yangın çıktı! 3 saniye gecikme.\n");
        sleep(3);
    }
    ```

---

## 🖥️ Örnek Program Çıktısı

```shell
[KAT 1] İnşaat başlıyor...
[KAT 1 - DAİRE 1] Kaba inşaat tamamlandı...
[KAT 1 - DAİRE 2] Vinç bekleniyor...
[YANGIN] Kat 1'de yangın! 3 saniye gecikme.
...
[KAT 10] Tamamlandı!
[BAHÇE] Peyzaj tamamlandı. Bina teslim edildi.
