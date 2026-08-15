#include "RC522.h"
// Định nghĩa một số thanh ghi cơ bản của MFRC522
#define CommandReg    0x01
#define ModeReg       0x11
#define TxControlReg  0x14
#define TxASKReg      0x15
uint8_t CardID[4];
uint8_t status;
uint8_t MasterCardID[4] = {0xBA,0x3D,0xF3,0x06};
uint8_t MemberCardList[5][5] =
    {
        {0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00}};
uint8_t MemberCardCount = 0;
uint8_t Master_Card_Exists = 0;
uint32_t last_activity_time = 0;
// Định nghĩa lệnh cơ bản
#define PCD_RESETPHASE 0x0F
MFRC522_Name MFRC522;
/**
 * @brief  So sánh hai ID thẻ RFID
 * @param  MFRC522: con trỏ đến cấu trúc MFRC522_Name
 * @param  CardID1: con trỏ đến ID thẻ thứ nhất
 * @param  CardID2: con trỏ đến ID thẻ thứ hai
 * @return 0 nếu giống nhau, 1 nếu khác nhau
 */
uint8_t MFRC522_Compare(MFRC522_Name* MFRC522, uint8_t *CardID1, uint8_t *CardID2) {
    for (uint8_t i = 0; i < 4; i++) {
        if (CardID1[i] != CardID2[i]) return 1;
    }
    return 0;
}
/*
 * Function Name: MFRC522_ReadRegister
 * Description: Read a byte from a specific register of the MFRC522
 * Input Parameters: MFRC522 - pointer to the MFRC522 structure; addr - register address
 * Returns: the value read from the register
 */
uint8_t MFRC522_ReadRegister(MFRC522_Name* MFRC522, uint8_t addr) {
    uint8_t val;
    // 1. Kéo CS xuống thấp để chọn chip
    HAL_GPIO_WritePin(MFRC522->CS_Port, MFRC522->CS_Pin, GPIO_PIN_RESET);
    
    // 2. Địa chỉ đọc: (addr << 1) | 0x80 (Bit 7 = 1 là lệnh đọc, Bit 0 = 0)
    uint8_t address_byte = ((addr << 1) & 0x7E) | 0x80;
    HAL_SPI_Transmit(MFRC522->hspi, &address_byte, 1, HAL_MAX_DELAY);
    
    // 3. Nhận dữ liệu trả về từ RC522
    HAL_SPI_Receive(MFRC522->hspi, &val, 1, HAL_MAX_DELAY);
    
    // 4. Kéo CS lên cao để kết thúc
    HAL_GPIO_WritePin(MFRC522->CS_Port, MFRC522->CS_Pin, GPIO_PIN_SET);
    
    return val;
}
// Hàm phụ trợ: Ghi dữ liệu vào thanh ghi RC522
void MFRC522_WriteRegister(MFRC522_Name* MFRC522, uint8_t addr, uint8_t val) {
    // Kéo chân CS xuống thấp để chọn RC522
    HAL_GPIO_WritePin(MFRC522->CS_Port, MFRC522->CS_Pin, GPIO_PIN_RESET);
    
    // Địa chỉ gửi đi: (addr << 1) & 0x7E (Bit 7 = 0 là lệnh ghi)
    uint8_t buffer[2];
    buffer[0] = (addr << 1) & 0x7E;
    buffer[1] = val;
    
    HAL_SPI_Transmit(MFRC522->hspi, buffer, 2, HAL_MAX_DELAY);
    
    // Kéo chân CS lên cao để kết thúc giao tiếp
    HAL_GPIO_WritePin(MFRC522->CS_Port, MFRC522->CS_Pin, GPIO_PIN_SET);
}
void MFRC522_ClearBitMask(MFRC522_Name* MFRC522, uint8_t reg, uint8_t mask)  
{
    uint8_t tmp;
    tmp = MFRC522_ReadRegister(MFRC522, reg);
    MFRC522_WriteRegister(MFRC522, reg, tmp & (~mask));  // clear bit mask
} 
/*
 * Function Name: SetBitMask
 * Description: Set RC522 register bit
 * Input parameters: reg - register address; mask - set value
 * Return value: None
 */
void MFRC522_SetBitMask(MFRC522_Name* MFRC522, uint8_t reg, uint8_t mask)  
{
    // MFRC522_ReadRegister là hàm đọc thanh ghi qua SPI bạn cần viết thêm
    uint8_t tmp = MFRC522_ReadRegister(MFRC522, reg);
    MFRC522_WriteRegister(MFRC522, reg, tmp | mask);  // Bật các bit trong mặt nạ mask
}
void AntennaOn(MFRC522_Name* MFRC522)
{
    uint8_t value = MFRC522_ReadRegister(MFRC522, TxControlReg);
    if ((value & 0x03) != 0x03) {
        MFRC522_SetBitMask(MFRC522, TxControlReg, 0x03); // Bật cả hai bit Tx1 và Tx2
    }
}   
void MFRC522_Reset(MFRC522_Name* MFRC522)
{
    MFRC522_WriteRegister(MFRC522, CommandReg, PCD_RESETPHASE);
}
// --- ĐÂY LÀ HÀM INIT BẠN CẦN ---
void MFRC522_Init(MFRC522_Name* MFRC522, SPI_HandleTypeDef* hspi, 
                  GPIO_TypeDef* CS_Port, uint16_t CS_Pin, 
                  GPIO_TypeDef* RST_Port, uint16_t RST_Pin) 
{
    // 1. Lưu cấu hình vào struct
    MFRC522->hspi = hspi;
    MFRC522->CS_Port = CS_Port;
    MFRC522->CS_Pin = CS_Pin;
    MFRC522->RST_Port = RST_Port;
    MFRC522->RST_Pin = RST_Pin;
    
    // 2. Kích hoạt chân Reset cứng và chân CS ban đầu
    HAL_GPIO_WritePin(MFRC522->CS_Port, MFRC522->CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MFRC522->RST_Port, MFRC522->RST_Pin, GPIO_PIN_SET); // Bật chip dậy
    
    // 3. Reset phần mềm
    MFRC522_Reset(MFRC522); 
    
    // 4. Cấu hình các thanh ghi (Sử dụng hàm WriteRegister của bạn)
    MFRC522_WriteRegister(MFRC522, TModeReg, 0x8D);      
    MFRC522_WriteRegister(MFRC522, TPrescalerReg, 0x3E); 
    MFRC522_WriteRegister(MFRC522, TReloadRegL, 30);           
    MFRC522_WriteRegister(MFRC522, TReloadRegH, 0);
    
    MFRC522_WriteRegister(MFRC522, TxAutoReg, 0x40);    
    MFRC522_WriteRegister(MFRC522, ModeReg, 0x3D);      

    // 5. Bật anten
    AntennaOn(MFRC522);
}
uchar MFRC522_ToCard(MFRC522_Name* MFRC522, uchar command, uchar *sendData, uchar sendLen, uchar *backData, uint *backLen)
{
    uchar status = MI_ERR;
    uchar irqEn = 0x00;
    uchar waitIRq = 0x00;
    uchar lastBits;
    uchar n;
    uint i;

    switch (command)
    {
        case PCD_AUTHENT:		// Certification cards close
		{
			irqEn = 0x12;
			waitIRq = 0x10;
			break;
		}
		case PCD_TRANSCEIVE:	// Transmit FIFO data
		{
			irqEn = 0x77;
			waitIRq = 0x30;
			break;
		}
		default:
			break;
    }
   
    MFRC522_WriteRegister(MFRC522, CommIEnReg, irqEn|0x80);	// Interrupt request
    MFRC522_ClearBitMask(MFRC522, CommIrqReg, 0x80);			// Clear all interrupt request bit
    MFRC522_SetBitMask(MFRC522, FIFOLevelReg, 0x80);			// FlushBuffer=1, FIFO Initialization
    
	MFRC522_WriteRegister(MFRC522, CommandReg, PCD_IDLE);	// NO action; Cancel the current command

	// Writing data to the FIFO
    for (i=0; i<sendLen; i++)
    {   
		MFRC522_WriteRegister(MFRC522, FIFODataReg, sendData[i]);    
	}

    // Execute the command
	MFRC522_WriteRegister(MFRC522, CommandReg, command);
    if (command == PCD_TRANSCEIVE)
    {    
		MFRC522_SetBitMask(MFRC522, BitFramingReg, 0x80);		// StartSend=1,transmission of data starts
	}   
    
    // Waiting to receive data to complete
	i = 2000;	// i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms
    do 
    {
		//CommIrqReg[7..0]
		//Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
        n = MFRC522_ReadRegister(MFRC522, CommIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitIRq));

    MFRC522_ClearBitMask(MFRC522, BitFramingReg, 0x80);			//StartSend=0
	
    if (i != 0)
    {    
        if(!(MFRC522_ReadRegister(MFRC522, ErrorReg) & 0x1B))	//BufferOvfl Collerr CRCErr ProtecolErr
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {   
				status = MI_ERR;
			}

            if (command == PCD_TRANSCEIVE)
            {
               	n = MFRC522_ReadRegister(MFRC522, FIFOLevelReg);
              	lastBits = MFRC522_ReadRegister(MFRC522, ControlReg) & 0x07;
                if (lastBits)
                {   
					*backLen = (n-1)*8 + lastBits;   
				}
                else
                {   
					*backLen = n*8;   
				}

                if (n == 0)
                {   
					n = 1;    
				}
                if (n > MAX_LEN)
                {   
					n = MAX_LEN;   
				}
				
                // Reading the received data in FIFO
                for (i=0; i<n; i++)
                {   
					backData[i] = MFRC522_ReadRegister(MFRC522, FIFODataReg);    
				}
            }
        }
        else
        {   
			status = MI_ERR;  
		}
        
    }
	
    //SetBitMask(ControlReg,0x80);           //timer stops
    //Write_MFRC522(CommandReg, PCD_IDLE); 

    return status;
}
/**
 * * Function Name: MFRC522_Request
 * @brief dùng lệnh để kiểm tra xem có thẻ RFID trong phạm vi hay không.
 * @param reqMode: chế độ yêu cầu
 * @param TagType: con trỏ đến mảng để lưu loại thẻ
 * @return MI_OK nếu có thẻ, MI_ERR nếu không
 * Description: Find cards, read the card type number
 * Input parameters: reqMode - find cards way
 *   TagType - Return Card Type
 *    0x4400 = Mifare_UltraLight
 *    0x0400 = Mifare_One(S50)
 *    0x0200 = Mifare_One(S70)
 *    0x0800 = Mifare_Pro(X)
 *    0x4403 = Mifare_DESFire
 * Return value: the successful return MI_OK
 */
uchar MFRC522_Request(MFRC522_Name* MFRC522, uchar reqMode, uchar *TagType)
{
	uchar status;  
	uint backBits;			 // The received data bits

	MFRC522_WriteRegister(MFRC522, BitFramingReg, 0x07);		//TxLastBists = BitFramingReg[2..0]
	
	TagType[0] = reqMode;
	status = MFRC522_ToCard(MFRC522, PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

	if ((status != MI_OK) || (backBits != 0x10))
	{    
		status = MI_ERR;
	}
   
	return status;
}
uchar MFRC522_Anticoll(MFRC522_Name* MFRC522, uchar *serNum)
{
    uchar status;
    uchar i;
	uchar serNumCheck=0;
    uint unLen;
    
	MFRC522_WriteRegister(MFRC522, BitFramingReg, 0x00);		//TxLastBists = BitFramingReg[2..0]
 
    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    status = MFRC522_ToCard(MFRC522, PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

    if (status == MI_OK)
	{
    	 //Check card serial number
		for (i=0; i<4; i++)
		{   
		 	serNumCheck ^= serNum[i];
		}
		if (serNumCheck != serNum[i])
		{   
			status = MI_ERR;    
		}
    }

    return status;
} 
uint8_t MFRC522_Check(MFRC522_Name *MFRC522, uint8_t *CardID)
{
    uint8_t TagType[2];         // lưu trữ 2 byte ATQA

    if (MFRC522_Request(MFRC522, PICC_REQIDL, TagType) != MI_OK)
        return MI_ERR;

    if (MFRC522_Anticoll(MFRC522, CardID) != MI_OK)
        return MI_ERR;

    return MI_OK;
}
void Flash_Buzzer_And_Led(uint8_t count, uint16_t on_ms, uint16_t off_ms) // hàm nhấp nháy led và buzzer* parameter count: số lần nhấp nháy, thơi gian delay
{
  // thiết lập trạng thái buzzer kêu và led lock sáng
  GPIO_PinState buzzer_active_state = GPIO_PIN_SET;
  GPIO_PinState led_active_state = GPIO_PIN_SET;
  for (uint8_t i = 0; i < count; i++)
  {
    // kích hoạt trạng thái
    HAL_GPIO_WritePin(Buzzes_GPIO_Port, Buzzes_Pin, buzzer_active_state);
    HAL_GPIO_WritePin(LED_LOCKdoor_GPIO_Port , LED_LOCKdoor_Pin, led_active_state);
    HAL_Delay(on_ms);
    // vô hiệu hóa trạng thái
    HAL_GPIO_WritePin(Buzzes_GPIO_Port, Buzzes_Pin, !buzzer_active_state);
    HAL_GPIO_WritePin(LED_LOCKdoor_GPIO_Port , LED_LOCKdoor_Pin, !led_active_state);
    HAL_Delay(off_ms);
  }
  // thiết lập buzzer và led trở vào trạng thái nghỉ
  HAL_GPIO_WritePin(LED_LOCKdoor_GPIO_Port , LED_LOCKdoor_Pin, led_active_state);
  HAL_GPIO_WritePin(Buzzes_GPIO_Port, Buzzes_Pin, !buzzer_active_state);
}
uint8_t Member(uint8_t *CardID, MFRC522_Name *MFRC522)
{
  for (int i = 0; i < MemberCardCount; i++)
  {
    if (MFRC522_Compare(MFRC522, CardID, MemberCardList[i]) == MI_OK)
    {
      return 1; // thẻ thành viên hợp lệ
    }
  }
  return 0; // không phải thẻ thành viên
}
uint8_t AddNewMemberOrMasterCard(uint8_t *CardID)
{
  // kiểm tra xem id này đã tồn tại trong master hoặc member chưa
  if (memcmp(CardID, MasterCardID, 4) == 0)
    return 2; // đã là thẻ master
  if (Member(CardID, &MFRC522))
    return 3; // đã là thẻ thành viên
  if (MemberCardCount >= 5)
  {
    Lcd_xoa_manhinh();
    HAL_Delay(20);
    Lcd_Ghi_Lenh(0x80);
    Lcd_Ghi_Chuoi(" KHONG THANH CONG");
    Lcd_Ghi_Lenh(0xC0);
    Lcd_Ghi_Chuoi(" TOI DA LA 05 THE");
    Flash_Buzzer_And_Led(3, 50, 50);
    HAL_Delay(2000);
    return 0;
  }
  // thêm thẻ mới
  memcpy(MemberCardList[MemberCardCount], CardID, 5);
  MemberCardCount++;
  return 1; // thêm thành công
}
uint8_t DeleteMemberCard(uint8_t *CardID)
{
  for (int i = 0; i < MemberCardCount; i++)
  {
    if (MFRC522_Compare(&MFRC522, CardID, MemberCardList[i]) == MI_OK)
    {
      // tìm thẻ, thay thế thẻ bị xóa bằng thẻ cuối cùng trong danh sách
      if (i < MemberCardCount - 1)
      {
        memcpy(MemberCardList[i], MemberCardList[MemberCardCount - 1], 5);
      }
      // giảm số lượng thẻ và reset id thẻ ở vị trí cuối cùng
      MemberCardCount--;
      memset(MemberCardList[MemberCardCount], 0x00, 5); // đặt id thẻ cuối cùng vào 0x00
      return 1;                                         // xóa thành công
    }
  }
  return 0; // thẻ không tồn tại
}
void Buzzer_Beep(uint16_t duration_ms)
{
  HAL_GPIO_WritePin(Buzzes_GPIO_Port, Buzzes_Pin, GPIO_PIN_SET);
  HAL_Delay(duration_ms);
  HAL_GPIO_WritePin(Buzzes_GPIO_Port, Buzzes_Pin, GPIO_PIN_RESET);
}
/**
 * @brief  Mở cửa khi quét thẻ thành công
 * @param  message: thông báo hiển thị trên LCD
 */
void Access_GrantedCard(char *message)
{
  Lcd_xoa_manhinh();
  Lcd_Ghi_Chuoi(message);
  Servo_SetAngle(150);
  HAL_GPIO_WritePin(LED_UnlockDoor_GPIO_Port,LED_UnlockDoor_Pin , GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_LOCKdoor_GPIO_Port , LED_LOCKdoor_Pin, GPIO_PIN_RESET);
  Buzzer_Beep(50);
  COUNTDOWN();
  Servo_SetAngle(0);
  HAL_GPIO_WritePin(LED_UnlockDoor_GPIO_Port,LED_UnlockDoor_Pin , GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_LOCKdoor_GPIO_Port , LED_LOCKdoor_Pin, GPIO_PIN_SET);
  CurrentState = STATE_LOCK_INPUT;
  Lcd_xoa_manhinh();
  Lcd_Ghi_Lenh(0x80);
  Lcd_Ghi_Chuoi("NHAP MAT KHAU: ");
  last_activity_time = HAL_GetTick();
}