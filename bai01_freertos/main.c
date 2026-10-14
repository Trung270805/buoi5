#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* =========================================================
 * 1. DINH NGHIA THANH GHI STM32F103C8T6
 * ========================================================= */

#define REG32(addr) (*(volatile uint32_t *)(addr))

#define RCC_APB2ENR REG32(0x40021018U)
#define RCC_APB1ENR REG32(0x4002101CU)

#define GPIOA_CRL   REG32(0x40010800U)

#define GPIOB_CRL   REG32(0x40010C00U)
#define GPIOB_CRH   REG32(0x40010C04U)
#define GPIOB_IDR   REG32(0x40010C08U)
#define GPIOB_ODR   REG32(0x40010C0CU)

#define GPIOC_CRH   REG32(0x40011004U)
#define GPIOC_ODR   REG32(0x4001100CU)

#define AFIO_MAPR   REG32(0x40010004U)

#define TIM2_CR1    REG32(0x40000000U)
#define TIM2_EGR    REG32(0x40000014U)
#define TIM2_CCMR1  REG32(0x40000018U)
#define TIM2_CCMR2  REG32(0x4000001CU)
#define TIM2_CCER   REG32(0x40000020U)
#define TIM2_PSC    REG32(0x40000028U)
#define TIM2_ARR    REG32(0x4000002CU)

#define TIM2_CCR1   REG32(0x40000034U)
#define TIM2_CCR2   REG32(0x40000038U)
#define TIM2_CCR3   REG32(0x4000003CU)

/* =========================================================
 * 2. CAU HINH CHAN
 * ========================================================= */

/*
 * LED PWM:
 * PA0 = TIM2_CH1
 * PA1 = TIM2_CH2
 * PA2 = TIM2_CH3
 *
 * Theo so do dau noi:
 * PAx -> dien tro han dong -> Anode LED
 * Cathode LED -> GND
 *
 * LED sang khi dau ra PWM o muc cao.
 */
#define LED_ACTIVE_LOW  0U

/*
 * Nut nhan:
 * PB0  = chon LED
 * PB1  = tang do sang
 * PB10 = giam do sang
 *
 * Moi nut noi GPIO voi GND khi nhan.
 * GPIO su dung dien tro pull-up noi.
 */
#define BTN_SELECT      (1U << 0)
#define BTN_INCREASE    (1U << 1)
#define BTN_DECREASE    (1U << 10)

#define BUTTON_COUNT        3U
#define LED_COUNT           3U

#define LED_STACK_SIZE      128U
#define BUTTON_STACK_SIZE   192U

#define BUTTON_SCAN_MS      10U
#define DEBOUNCE_COUNT      3U

#define BRIGHTNESS_STEP     10U
#define INITIAL_BRIGHTNESS  40U

#define LED_PC13            (1U << 13)

/* =========================================================
 * 3. CAU TRUC DU LIEU
 * ========================================================= */

/* Moi LED co mot queue va mot thanh ghi CCR rieng */
typedef struct
{
    QueueHandle_t queue;
    volatile uint32_t *ccr;
} LEDTaskConfig;

/* Ba queue, moi queue chua mot gia tri do sang */
static QueueHandle_t ledQueues[LED_COUNT];

/* Cau hinh rieng cho tung LED task */
static LEDTaskConfig ledTaskConfig[LED_COUNT];

/* Do sang ban dau: 40% cho moi LED */
static uint8_t ledBrightness[LED_COUNT] =
{
    INITIAL_BRIGHTNESS,
    INITIAL_BRIGHTNESS,
    INITIAL_BRIGHTNESS
};

/* LED duoc chon de tang/giam do sang */
static uint8_t selectedLed = 0U;

/* =========================================================
 * 4. KHOI TAO PHAN CUNG
 * ========================================================= */

static void Hardware_Init(void)
{
    /*
     * Bat clock:
     * AFIO = bit 0
     * GPIOA = bit 2
     * GPIOB = bit 3
     * GPIOC = bit 4
     */
    RCC_APB2ENR |= (1U << 0)
                |  (1U << 2)
                |  (1U << 3)
                |  (1U << 4);

    /* Bat clock TIM2 */
    RCC_APB1ENR |= (1U << 0);

    /*
     * Su dung mapping mac dinh cua TIM2:
     * CH1 -> PA0
     * CH2 -> PA1
     * CH3 -> PA2
     */
    AFIO_MAPR &= ~(3U << 8);

    /*
     * PA0, PA1, PA2:
     * Alternate Function Push-Pull, 50 MHz.
     */
    GPIOA_CRL &= ~0xFFFU;
    GPIOA_CRL |=  0xBBBU;

    /*
     * PB0, PB1: Input Pull-up/Pull-down.
     * Moi chan chiem 4 bit trong GPIOB_CRL.
     */
    GPIOB_CRL &= ~0xFFU;
    GPIOB_CRL |=  0x88U;

    /*
     * PB10: Input Pull-up/Pull-down.
     * PB10 nam o nibble thu 2 cua GPIOB_CRH.
     */
    GPIOB_CRH &= ~(0xFU << 8);
    GPIOB_CRH |=  (0x8U << 8);

    /* Chon pull-up noi cho ca ba nut */
    GPIOB_ODR |= BTN_SELECT
               | BTN_INCREASE
               | BTN_DECREASE;

    /*
     * PC13: GPIO Output Push-Pull, 2 MHz.
     * PC13 se dao trang thai moi khi nhan nut hop le.
     */
    GPIOC_CRH &= ~(0xFU << 20);
    GPIOC_CRH |=  (0x2U << 20);

    /* Tat LED onboard PC13 (active-low) */
    GPIOC_ODR |= LED_PC13;

    /*
     * TIM2 PWM:
     *
     * Mac dinh HSI = 8 MHz.
     * PSC = 7  -> tan so dem 1 MHz.
     * ARR = 999 -> PWM 1 kHz.
     *
     * Cau hinh nay dung khi clock TIM2 la 8 MHz.
     */
    TIM2_CR1 = 0U;

    TIM2_PSC = 7U;
    TIM2_ARR = 999U;

    /*
     * CH1 va CH2:
     * PWM Mode 1, Output Compare Preload Enable.
     */
    TIM2_CCMR1 = 0x6868U;

    /*
     * CH3:
     * PWM Mode 1, Output Compare Preload Enable.
     */
    TIM2_CCMR2 = 0x0068U;

    /* Cho phep output CH1, CH2, CH3 */
    TIM2_CCER = 0x0111U;

    /* Ban dau tat ca LED */
    TIM2_CCR1 = 0U;
    TIM2_CCR2 = 0U;
    TIM2_CCR3 = 0U;

    /* Nap cac thanh ghi timer */
    TIM2_EGR = 1U;

    /* Bat preload ARR va bat TIM2 */
    TIM2_CR1 = (1U << 7) | (1U << 0);
}

/* =========================================================
 * 5. GUI DO SANG VAO QUEUE
 * ========================================================= */

static void Publish_LED_Brightness(void)
{
    uint8_t i;

    for (i = 0U; i < LED_COUNT; i++)
    {
        if (ledQueues[i] != NULL)
        {
            /*
             * Queue co do dai 1.
             * Ghi de gia tri cu bang gia tri moi.
             */
            xQueueOverwrite(
                ledQueues[i],
                &ledBrightness[i]
            );
        }
    }
}

/* =========================================================
 * 6. TINH GIA TRI COMPARE CHO PWM
 * ========================================================= */

static uint32_t Brightness_To_Compare(uint8_t brightness)
{
    uint32_t compareValue;

    if (brightness > 100U)
    {
        brightness = 100U;
    }

#if LED_ACTIVE_LOW

    /*
     * Neu LED sang khi chan PA o muc thap,
     * dao nguoc duty cycle.
     */
    compareValue =
        ((uint32_t)(100U - brightness) * 1000U) / 100U;

#else

    /*
     * Neu LED sang khi chan PA o muc cao,
     * duty cycle bang phan tram do sang.
     */
    compareValue =
        ((uint32_t)brightness * 1000U) / 100U;

#endif

    return compareValue;
}

/* =========================================================
 * 7. TASK DIEU KHIEN LED
 * ========================================================= */

static void LED_Task(void *pvParameters)
{
    LEDTaskConfig *config;
    uint8_t brightness;
    uint32_t compareValue;

    config = (LEDTaskConfig *)pvParameters;

    for (;;)
    {
        /*
         * Cho nhan gia tri do sang tu queue.
         * Moi LED task chi doc queue cua chinh no.
         */
        if (xQueueReceive(
                config->queue,
                &brightness,
                portMAX_DELAY
            ) == pdPASS)
        {
            compareValue =
                Brightness_To_Compare(brightness);

            /* Cap nhat duty cycle PWM */
            *(config->ccr) = compareValue;
        }
    }
}

/* =========================================================
 * 8. XU LY SU KIEN NUT NHAN
 * ========================================================= */

static void Handle_Button_Press(uint8_t buttonIndex)
{
    /*
     * Dao trang thai PC13 de bao hieu
     * mot su kien nhan nut da duoc ghi nhan.
     */
    GPIOC_ODR ^= LED_PC13;

    if (buttonIndex == 0U)
    {
        /*
         * PB0: Chon LED tiep theo.
         *
         * LED 1 -> LED 2 -> LED 3 -> LED 1.
         */
        selectedLed++;

        if (selectedLed >= LED_COUNT)
        {
            selectedLed = 0U;
        }
    }
    else if (buttonIndex == 1U)
    {
        /* PB1: Tang do sang LED dang duoc chon */

        if (ledBrightness[selectedLed] < 100U)
        {
            if (ledBrightness[selectedLed] >
                (100U - BRIGHTNESS_STEP))
            {
                ledBrightness[selectedLed] = 100U;
            }
            else
            {
                ledBrightness[selectedLed] +=
                    BRIGHTNESS_STEP;
            }
        }
    }
    else if (buttonIndex == 2U)
    {
        /* PB10: Giam do sang LED dang duoc chon */

        if (ledBrightness[selectedLed] >=
            BRIGHTNESS_STEP)
        {
            ledBrightness[selectedLed] -=
                BRIGHTNESS_STEP;
        }
        else
        {
            ledBrightness[selectedLed] = 0U;
        }
    }

    /*
     * Gui cac gia tri do sang moi den queue.
     * Ba LED task se tu nhan gia tri tu queue rieng.
     */
    Publish_LED_Brightness();
}

/* =========================================================
 * 9. TASK DOC NUT NHAN
 * ========================================================= */

static void Button_Task(void *pvParameters)
{
    (void)pvParameters;

    const uint32_t buttonMasks[BUTTON_COUNT] =
    {
        BTN_SELECT,
        BTN_INCREASE,
        BTN_DECREASE
    };

    /*
     * 1 = nut dang tha.
     * 0 = nut dang duoc nhan.
     */
    uint8_t candidate[BUTTON_COUNT] =
    {
        1U, 1U, 1U
    };

    uint8_t stable[BUTTON_COUNT] =
    {
        1U, 1U, 1U
    };

    uint8_t debounceCount[BUTTON_COUNT] =
    {
        0U, 0U, 0U
    };

    uint32_t gpioInput;
    uint8_t rawState;
    uint8_t i;

    for (;;)
    {
        /* Doc trang thai GPIOB */
        gpioInput = GPIOB_IDR;

        for (i = 0U; i < BUTTON_COUNT; i++)
        {
            /*
             * Pull-up:
             * Tha nut  -> GPIO = 1.
             * Nhan nut -> GPIO = 0.
             */
            rawState =
                ((gpioInput & buttonMasks[i]) != 0U)
                ? 1U : 0U;

            /*
             * Neu trang thai thay doi,
             * bat dau dem debounce lai.
             */
            if (rawState != candidate[i])
            {
                candidate[i] = rawState;
                debounceCount[i] = 0U;
            }
            else
            {
                if (debounceCount[i] < DEBOUNCE_COUNT)
                {
                    debounceCount[i]++;
                }
            }

            /*
             * Chap nhan trang thai moi sau
             * nhieu lan doc lien tiep on dinh.
             */
            if ((debounceCount[i] >= DEBOUNCE_COUNT) &&
                (stable[i] != candidate[i]))
            {
                stable[i] = candidate[i];

                /*
                 * Chi xu ly khi nut vua duoc nhan xuong.
                 * Khi giu nut, khong lap lai su kien.
                 */
                if (stable[i] == 0U)
                {
                    Handle_Button_Press(i);
                }
            }
        }

        /* Quet nut moi 10 ms */
        vTaskDelay(pdMS_TO_TICKS(BUTTON_SCAN_MS));
    }
}

/* =========================================================
 * 10. HAM MAIN
 * ========================================================= */

int main(void)
{
    uint8_t i;

    /* Khoi tao GPIO va TIM2 */
    Hardware_Init();

    /*
     * Tao ba queue, moi queue:
     * - Do dai: 1 phan tu.
     * - Kich thuoc phan tu: 1 byte.
     */
    for (i = 0U; i < LED_COUNT; i++)
    {
        ledQueues[i] =
            xQueueCreate(1U, sizeof(uint8_t));

        if (ledQueues[i] == NULL)
        {
            for (;;)
            {
                /* Dung neu khong tao duoc queue */
            }
        }
    }

    /* Cau hinh LED 1: PA0 / TIM2_CH1 */
    ledTaskConfig[0].queue = ledQueues[0];
    ledTaskConfig[0].ccr = &TIM2_CCR1;

    /* Cau hinh LED 2: PA1 / TIM2_CH2 */
    ledTaskConfig[1].queue = ledQueues[1];
    ledTaskConfig[1].ccr = &TIM2_CCR2;

    /* Cau hinh LED 3: PA2 / TIM2_CH3 */
    ledTaskConfig[2].queue = ledQueues[2];
    ledTaskConfig[2].ccr = &TIM2_CCR3;

    /* Tao 3 task LED doc lap */
    for (i = 0U; i < LED_COUNT; i++)
    {
        if (xTaskCreate(
                LED_Task,
                "LED",
                LED_STACK_SIZE,
                &ledTaskConfig[i],
                1U,
                NULL
            ) != pdPASS)
        {
            for (;;)
            {
                /* Dung neu tao task that bai */
            }
        }
    }

    /* Tao task doc ca ba nut nhan */
    if (xTaskCreate(
            Button_Task,
            "Buttons",
            BUTTON_STACK_SIZE,
            NULL,
            2U,
            NULL
        ) != pdPASS)
    {
        for (;;)
        {
            /* Dung neu tao task that bai */
        }
    }

    /* Gui do sang khoi tao cho ba LED */
    Publish_LED_Brightness();

    /* Bat dau FreeRTOS */
    vTaskStartScheduler();

    /*
     * Neu chuong trinh den day,
     * scheduler khong khoi dong duoc.
     */
    for (;;)
    {
    }
}
