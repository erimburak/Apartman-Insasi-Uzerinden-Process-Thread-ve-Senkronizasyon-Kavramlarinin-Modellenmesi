// Gerekli kütüphaneler dahil ediliyor
#include <stdio.h>      // Giriş/Çıkış işlemleri için
#include <stdlib.h>     // Temel işlemler (malloc, exit, vs.)
#include <unistd.h>     // UNIX fonksiyonları (fork, sleep, vs.)
#include <pthread.h>    // Thread işlemleri için
#include <semaphore.h>  // Semafor (eş zamanlılık kontrolü) için
#include <sys/mman.h>   // Paylaşımlı bellek kullanımı için
#include <sys/wait.h>   // Çocuk proseslerin takibi için
#include <time.h>       // Rastgelelik ve zaman ölçümleri için

// Bina ve kat yapılandırmaları
#define TOTAL_FLOORS 10         // Toplam kat sayısı
#define FLATS_PER_FLOOR 4       // Her katta daire sayısı

// Terminal renklendirme için makrolar
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"

// Kaynaklar için mutex tanımları (kilitler)
pthread_mutex_t electricity_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t water_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t crane_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t furniture_mutex = PTHREAD_MUTEX_INITIALIZER;

// Asansör için semafor tanımı (2 kişi kullanabilir)
sem_t elevator_sem;

// Kat durumlarını tutan paylaşımlı bellek işaretçisi
int* kat_durumu;

// Kat tamamlandığında bina çizimi için sayaç (şimdilik kullanılmıyor)
int tamamlanan_kat = 0;

// Rastgele usta profilleri dizisi
const char* usta_profilleri[] = {"Yavaş", "Tecrübeli", "Dalga Geçen"};

// ASCII bina çizimi yapan fonksiyon
void draw_ascii_building() {
    printf(BOLD MAGENTA "\nBina İnşaat Durumu:\n" RESET);
    for (int i = TOTAL_FLOORS; i >= 1; i--) {
        if (kat_durumu[i]) {
            printf(BOLD GREEN "[==== KAT %2d ====]\n" RESET, i); // Kat tamamlandıysa yeşil kutu
        } else {
            printf("[              ]\n"); // Kat tamamlanmadıysa boşluk
        }
    }
    printf("\n");
}

// Her daire için kullanılacak parametreleri tutan yapı
typedef struct {
    int floor; // Kat numarası
    int flat;  // Daire numarası
} FlatArgs;

// Ortak vinç kullanımı - mutual exclusion (mutex)
void use_shared_crane(int floor, int flat) {
    pthread_mutex_lock(&crane_mutex); // Vinci kilitle
    printf(CYAN "   [KAT %d - DAIRE %d] 🚧 Vinç kullanılıyor...\n" RESET, floor, flat);
    sleep(1); // Kullanım süresi
    pthread_mutex_unlock(&crane_mutex); // Vinci serbest bırak
}

// Asansör kullanımı - semafor ile sınırlı erişim
void use_elevator(int floor, int flat) {
    sem_wait(&elevator_sem); // Asansör semaforu azalt (erişim sağla)
    printf(YELLOW "   [KAT %d - DAIRE %d] 🛗 Asansör kullanılıyor (mobilya için)...\n" RESET, floor, flat);
    sleep(2); // Asansör kullanım süresi
    printf(YELLOW "   [KAT %d - DAIRE %d] 🛋️ Mobilya çıkarıldı, asansör serbest.\n" RESET, floor, flat);
    sem_post(&elevator_sem); // Asansör semaforunu artır (erişimi serbest bırak)
}

// Rastgele gecikme ve sorunlar oluşturur
void handle_random_events(int floor, int flat) {
    int event = rand() % 10; // 0–9 arası rastgele olay
    switch (event) {
        case 0:
            printf(RED "   [KAT %d - DAIRE %d] 🧯 Yangın alarmı! Acil müdahale...\n" RESET, floor, flat);
            sleep(3); // Müdahale süresi
            break;
        case 1:
            printf(CYAN "   [KAT %d - DAIRE %d] 🧊 Malzeme donması nedeniyle bekleniyor...\n" RESET, floor, flat);
            sleep(2);
            break;
        case 2:
            printf(MAGENTA "   [KAT %d - DAIRE %d] 📦 Yanlış malzeme teslimatı! İade süreci...\n" RESET, floor, flat);
            sleep(2);
            break;
        case 3:
            printf(YELLOW "   [KAT %d - DAIRE %d] 🧑‍🔧 Usta grevi! Çırak devrede...\n" RESET, floor, flat);
            sleep(2);
            break;
        case 4:
            printf(BLUE "   [KAT %d - DAIRE %d] 💧 Su kaçağı tespit edildi, tamir ediliyor...\n" RESET, floor, flat);
            sleep(2);
            break;
        default:
            break; // Diğer olasılık: bir şey olmaz
    }
}

// Her daire inşaat işlemlerini yürüten thread fonksiyonu
void* build_flat(void* args) {
    FlatArgs* flat_args = (FlatArgs*)args;
    int floor = flat_args->floor;
    int flat = flat_args->flat;

    sleep(1); // Kaba inşaat süresi (simülasyon)

    // Malzeme tedarikinde gecikme
    int delay = rand() % 3 + 1;
    printf(MAGENTA "   [KAT %d - DAIRE %d] Malzeme tedariki bekleniyor (%d saniye)...\n" RESET, floor, flat, delay);
    sleep(delay);

    // Rastgele sorunlar
    handle_random_events(floor, flat);

    // Elektrik tesisatı (mutex ile korunuyor)
    pthread_mutex_lock(&electricity_mutex);
    printf(BLUE "   [KAT %d - DAIRE %d] ⚡ Elektrik tesisatı yapılıyor...\n" RESET, floor, flat);
    sleep(1);
    pthread_mutex_unlock(&electricity_mutex);

    // Su tesisatı (mutex ile korunuyor)
    pthread_mutex_lock(&water_mutex);
    printf(BLUE "   [KAT %d - DAIRE %d] 💧 Su tesisatı yapılıyor...\n" RESET, floor, flat);
    sleep(1);
    pthread_mutex_unlock(&water_mutex);

    // Vinç ile ağır malzeme taşınması
    use_shared_crane(floor, flat);

    // Boya ve sıva işlemleri
    printf(GREEN "   [KAT %d - DAIRE %d] 🎨 Boya ve sıva yapılıyor...\n" RESET, floor, flat);
    sleep(1);
    printf(GREEN "   [KAT %d - DAIRE %d] 🎨 Boya ve sıva tamamlandı.\n" RESET, floor, flat);

    // Mobilya taşıma işlemi (asansör ile)
    use_elevator(floor, flat);

    // Mobilya yerleştirme (mutex ile korunuyor)
    pthread_mutex_lock(&furniture_mutex);
    printf(CYAN "   [KAT %d - DAIRE %d] 🛋️ Mobilya yerleştiriliyor...\n" RESET, floor, flat);
    sleep(1);
    pthread_mutex_unlock(&furniture_mutex);
    printf(CYAN "   [KAT %d - DAIRE %d] 🛋️ Mobilya yerleştirme tamamlandı.\n" RESET, floor, flat);

    // Rastgele usta profili yazdırma
    const char* profil = usta_profilleri[rand() % 3];
    printf(WHITE "   [KAT %d - DAIRE %d] 👷 Usta profili: %s\n" RESET, floor, flat, profil);

    free(flat_args); // Bellek temizleme
    return NULL;
}

// Bahçe ve dış mekan dekorasyon işlemleri
void build_garden_decorations() {
    printf(BOLD "[DIS MEKAN] 🌳 Bahçe süsleri, havuz ve şelale yapılıyor...\n" RESET);
    sleep(3); // Dekorasyon süresi
    printf(BOLD "[DIS MEKAN] 🌳 Bahçe süsleri tamamlandı.\n" RESET);
}

// Her kat için süreçleri yöneten fonksiyon
void build_floor(int floor) {
    // Önceki kat tamamlanmadan bu kat başlamasın
    while (floor > 1 && kat_durumu[floor - 1] == 0) {
        usleep(100000); // 100ms bekle
    }

    // Kat inşaat başlatılıyor
    printf(BOLD "============================================\n" RESET);
    printf(BOLD BLUE "             [KAT %d] İnşaata başlanıyor\n" RESET, floor);
    printf(BOLD "============================================\n" RESET);

    // Malzeme gecikmesi simülasyonu
    int delay = rand() % 3;
    if (delay == 0) {
        printf(RED "[KAT %d] Malzeme gecikti, bekleniyor...\n" RESET, floor);
        sleep(2);
    }

    sleep(2); // Katın kaba inşaatı
    kat_durumu[floor] = 1; // Kat tamamlandı olarak işaretleniyor
    draw_ascii_building(); // Güncel bina durumu çiziliyor
    printf(GREEN "[KAT %d] Kaba inşaat tamamlandı.\n" RESET, floor);

    // Kat içindeki daireleri inşa et
    pthread_t threads[FLATS_PER_FLOOR];
    for (int i = 0; i < FLATS_PER_FLOOR; i++) {
        FlatArgs* args = malloc(sizeof(FlatArgs));
        args->floor = floor;
        args->flat = i + 1;
        pthread_create(&threads[i], NULL, build_flat, args);
    }
    for (int i = 0; i < FLATS_PER_FLOOR; i++) {
        pthread_join(threads[i], NULL); // Tüm daireler bitmeden çıkma
    }

    printf(BOLD GREEN "[KAT %d] Tüm daireler tamamlandı.\n" RESET, floor);
    exit(0); // Kat prosesi sonlandırılır
}

// Ana fonksiyon - bina inşaatının başlatıldığı yer
int main() {
    srand(time(NULL)); // Rastgelelik için seed

    // Paylaşımlı bellek alanı ayarlanıyor
    kat_durumu = mmap(NULL, sizeof(int) * (TOTAL_FLOORS + 1),
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (kat_durumu == MAP_FAILED) {
        perror("Shared memory oluşturulamadı");
        exit(EXIT_FAILURE);
    }

    // Tüm katlar henüz tamamlanmamış olarak işaretleniyor
    for (int i = 0; i <= TOTAL_FLOORS; i++) {
        kat_durumu[i] = 0;
    }

    sem_init(&elevator_sem, 1, 2); // Asansör semaforu başlatılıyor

    // Her kat için ayrı bir proses oluşturuluyor
    for (int i = 1; i <= TOTAL_FLOORS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            build_floor(i); // Çocuk proses katı inşa eder
        }
    }

    // Ana proses tüm çocukları bekler
    for (int i = 1; i <= TOTAL_FLOORS; i++) {
        wait(NULL);
    }

    // Bahçe inşaatı en son yapılır
    build_garden_decorations();

    // İnşaat tamamlandı mesajı
    printf(BOLD MAGENTA "\nTÜM BİNA VE ÇEVRESİ TAMAMLANDI! 🎉\n" RESET);

    // Kaynaklar serbest bırakılır
    munmap(kat_durumu, sizeof(int) * (TOTAL_FLOORS + 1));
    sem_destroy(&elevator_sem);
    return 0;
}
