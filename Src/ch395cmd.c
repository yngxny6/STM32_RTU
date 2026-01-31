/**
 ****************************************************************************************************
 * @file        ch395cmd.C
 * @author      ÕıµãÔ­×ÓÍÅ¶Ó(ALIENTEK)
 * @version     V1.0
 * @date        2022-6-17
 * @brief       ch395ÃüÁî½Ó¿ÚÎÄ¼ş
 * @license     Copyright (c) 2020-2032, ¹ãÖİÊĞĞÇÒíµç×Ó¿Æ¼¼ÓĞÏŞ¹«Ë¾
 ****************************************************************************************************
 * @attention
 *
 * ÊµÑéÆ½Ì¨:ÕıµãÔ­×Ó STM32F103¿ª·¢°å
 * ÔÚÏßÊÓÆµ:www.yuanzige.com
 * ¼¼ÊõÂÛÌ³:www.openedv.com
 * ¹«Ë¾ÍøÖ·:www.alientek.com
 * ¹ºÂòµØÖ·:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "ch395inc.h"
#include "ch395cmd.h"
#include "ch395.h"
#include "main.h" // ä¸ºäº†ä½¿ç”¨ HAL_Delay

#define delay_ms(x) HAL_Delay(x)
#define delay_us(x) // å¾®ç§’å»¶æ—¶ï¼ŒHALåº“ä¸å¥½åšï¼Œä½†CH395é€Ÿåº¦å¿«ï¼Œé€šå¸¸å¯ä»¥å¿½ç•¥ï¼Œæˆ–è€…ç”¨ç©ºå¾ªç¯

#ifndef delay_us
#define delay_us(x)  // å¾®ç§’å»¶æ—¶ç›´æ¥å¿½ç•¥ï¼ŒCH395 é€Ÿåº¦å¤Ÿå¿«
#endif

/**
 * @brief       ¸´Î»ch395Ğ¾Æ¬
 * @param       ÎŞ
 * @retval      ÎŞ
 */
void ch395_cmd_reset(void)
{
    ch395_write_cmd(CMD00_RESET_ALL);
    ch395_scs_hign;
}

/**
 * @brief       Ê¹ch395½øÈëË¯Ãß×´Ì¬
 * @param       ÎŞ
 * @retval      ÎŞ
 */
void ch395_cmd_sleep(void)
{
    ch395_write_cmd(CMD00_ENTER_SLEEP);
    ch395_scs_hign;
}

/**
 * @brief       »ñÈ¡Ğ¾Æ¬ÒÔ¼°¹Ì¼ş°æ±¾ºÅ£¬1×Ö½Ú£¬¸ßËÄÎ»±íÊ¾Ğ¾Æ¬°æ±¾£¬
 * @param       ÎŞ
 * @retval      1×Ö½ÚĞ¾Æ¬¼°¹Ì¼ş°æ±¾ºÅ
 */
uint8_t ch395_cmd_get_ver(void)
{
    uint8_t i;
    ch395_write_cmd(CMD01_GET_IC_VER);
    i = ch395_read_data();
    ch395_scs_hign;
    return i;
}

/**
 * @brief       ²âÊÔÃüÁî£¬ÓÃÓÚ²âÊÔÓ²¼şÒÔ¼°½Ó¿ÚÍ¨Ñ¶£¬
 * @param       1×Ö½Ú²âÊÔÊı¾İ
 * @retval      Ó²¼şok£¬·µ»Ø testdata°´Î»È¡·´
 */
uint8_t ch395_cmd_check_exist(uint8_t testdata)
{
    uint8_t i;

    ch395_write_cmd(CMD11_CHECK_EXIST);
    ch395_write_data(testdata);
    i = ch395_read_data();
    ch395_scs_hign;
    return i;
}

/**
 * @brief       ÉèÖÃphy£¬Ö÷ÒªÉèÖÃch395 phyÎª100/10m »òÕßÈ«Ë«¹¤°ëË«¹¤£¬ch395Ä¬Îª×Ô¶¯Ğ­ÉÌ¡£
 * @param       ²Î¿¼phy ÃüÁî²ÎÊı/×´Ì¬
 * @retval      ÎŞ
 */
void ch395_cmd_set_phy(uint8_t phystat)
{
    ch395_write_cmd(CMD10_SET_PHY);
    ch395_write_data(phystat);
    ch395_scs_hign;
}

/**
 * @brief       »ñÈ¡phyµÄ×´Ì¬
 * @param       ÎŞ
 * @retval      µ±Ç°ch395phy×´Ì¬£¬²Î¿¼phy²ÎÊı/×´Ì¬¶¨Òå
 */
uint8_t ch395_cmd_get_phy_status(void)
{
    uint8_t i;

    ch395_write_cmd(CMD01_GET_PHY_STATUS);
    i = ch395_read_data();
    ch395_scs_hign;
    return i;
}

/**
 * @brief       »ñÈ¡È«¾ÖÖĞ¶Ï×´Ì¬£¬ÊÕµ½´ËÃüÁîch395×Ô¶¯È¡ÏûÖĞ¶Ï£¬0x43¼°ÒÔÏÂ°æ±¾Ê¹ÓÃ
 * @param       ÎŞ
 * @retval      ·µ»Øµ±Ç°µÄÈ«¾ÖÖĞ¶Ï×´Ì¬
 */
uint8_t ch395_cmd_get_glob_int_status(void)
{
    uint8_t init_status;

    ch395_write_cmd(CMD01_GET_GLOB_INT_STATUS);
    init_status = ch395_read_data();
    ch395_scs_hign;
    return  init_status;
}

/**
 * @brief       ³õÊ¼»¯ch395Ğ¾Æ¬
 * @param       ÎŞ
 * @retval      ·µ»ØÖ´ĞĞ½á¹û
 */
uint8_t ch395_cmd_init(void)
{
    uint8_t i = 0;
    uint8_t s = 0;

    ch395_write_cmd(CMD0W_INIT_CH395);
    ch395_scs_hign;

    while (1)
    {
        delay_ms(10);                          /* ÑÓÊ±²éÑ¯£¬½¨Òé2MSÒÔÉÏ */
        s = ch395_get_cmd_status();            /* ²»ÄÜ¹ıÓÚÆµ·±²éÑ¯ */

        if (s != CH395_ERR_BUSY)
        {
            break;         /* Èç¹ûCH395Ğ¾Æ¬·µ»ØÃ¦×´Ì¬ */
        }

        if (i++ > 200)
        {
            return CH395_ERR_UNKNOW; /* ³¬Ê±ÍË³ö,±¾º¯ÊıĞèÒª500MSÒÔÉÏÖ´ĞĞÍê±Ï */
        }
    }

    return s;
}

/**
 * @brief       ÉèÖÃch395´®¿Ú²¨ÌØÂÊ£¬½öÔÚ´®¿ÚÄ£Ê½ÏÂÓĞĞ§
 * @param       baudrate ´®¿Ú²¨ÌØÂÊ
 * @retval      ÎŞ
 */
void ch395_cmd_set_uart_baud_rate(uint32_t baudrate)
{
    ch395_write_cmd(CMD31_SET_BAUDRATE);
    ch395_write_data((uint8_t)baudrate);
    ch395_write_data((uint8_t)((uint16_t)baudrate >> 8));
    ch395_write_data((uint8_t)(baudrate >> 16));
    uint8_t i = ch395_read_data();
    ch395_scs_hign;
}

/**
 * @brief       »ñÈ¡ÃüÁîÖ´ĞĞ×´Ì¬£¬Ä³Ğ©ÃüÁîĞèÒªµÈ´ıÃüÁîÖ´ĞĞ½á¹û
 * @param       ÎŞ
 * @retval      ·µ»ØÉÏÒ»ÌõÃüÁîÖ´ĞĞ×´Ì¬
 */
uint8_t ch395_get_cmd_status(void)
{
    uint8_t i;

    ch395_write_cmd(CMD01_GET_CMD_STATUS);
    i = ch395_read_data();
    ch395_scs_hign;
    return i;
}

/**
 * @brief       ÉèÖÃch395µÄipµØÖ·
 * @param       ipaddr Ö¸ipµØÖ·
 * @retval      ÎŞ
 */
void ch395_cmd_set_ipaddr(uint8_t *ipaddr)
{
    uint8_t i;

    ch395_write_cmd(CMD40_SET_IP_ADDR);

    for (i = 0; i < 4; i++)
    {
        ch395_write_data(*ipaddr++);
    }

    ch395_scs_hign;
}

/**
 * @brief       ÉèÖÃch395µÄÍø¹ØipµØÖ·
 * @param       ipaddr Ö¸ÏòÍø¹ØipµØÖ·
 * @retval      ÎŞ
 */
void ch395_cmd_set_gw_ipaddr(uint8_t *gwipaddr)
{
    uint8_t i;

    ch395_write_cmd(CMD40_SET_GWIP_ADDR);

    for (i = 0; i < 4; i++)
    {
        ch395_write_data(*gwipaddr++);
    }

    ch395_scs_hign;
}

/**
 * @brief       ÉèÖÃch395µÄ×ÓÍøÑÚÂë£¬Ä¬ÈÏÎª255.255.255.0
 * @param       maskaddr Ö¸×ÓÍøÑÚÂëµØÖ·
 * @retval      ÎŞ
 */
void ch395_cmd_set_maskaddr(uint8_t *maskaddr)
{
    uint8_t i;

    ch395_write_cmd(CMD40_SET_MASK_ADDR);

    for (i = 0; i < 4; i++)
    {
        ch395_write_data(*maskaddr++);
    }

    ch395_scs_hign;
}

/**
 * @brief       ÉèÖÃch395µÄmacµØÖ·¡£
 * @param       mcaddr macµØÖ·Ö¸Õë
 * @retval      ÎŞ
 */
void ch395_cmd_set_macaddr(uint8_t *amcaddr)
{
    uint8_t i;

    ch395_write_cmd(CMD60_SET_MAC_ADDR);

    for (i = 0; i < 6; i++)
    {
        ch395_write_data(*amcaddr++);
    }

    ch395_scs_hign;
    delay_ms(100);
}

/**
 * @brief       »ñÈ¡ch395µÄmacµØÖ·¡£
 * @param       amcaddr macµØÖ·Ö¸Õë
 * @retval      ÎŞ
 */
void ch395_cmd_get_macaddr(uint8_t *amcaddr)
{
    uint8_t i;

    ch395_write_cmd(CMD06_GET_MAC_ADDR);

    for (i = 0; i < 6; i++)
    {
        *amcaddr++ = ch395_read_data();
    }

    ch395_scs_hign;
}

/**
 * @brief       ÉèÖÃmac¹ıÂË¡£
 * @param       filtype ²Î¿¼ mac¹ıÂË
 * @param       table0 hash0
 * @param       table1 hash1
 * @retval      ÎŞ
 */
void ch395_cmd_set_macfilt(uint8_t filtype, uint32_t table0, uint32_t table1)
{
    ch395_write_cmd(CMD90_SET_MAC_FILT);
    ch395_write_data(filtype);
    ch395_write_data((uint8_t)table0);
    ch395_write_data((uint8_t)((uint16_t)table0 >> 8));
    ch395_write_data((uint8_t)(table0 >> 16));
    ch395_write_data((uint8_t)(table0 >> 24));

    ch395_write_data((uint8_t)table1);
    ch395_write_data((uint8_t)((uint16_t)table1 >> 8));
    ch395_write_data((uint8_t)(table1 >> 16));
    ch395_write_data((uint8_t)(table1 >> 24));
    ch395_scs_hign;
}

/**
 * @brief       »ñÈ¡²»¿É´ïĞÅÏ¢ (ip,port,protocol type)
 * @param       list ±£´æ»ñÈ¡µ½µÄ²»¿É´ï
     @arg       µÚ1¸ö×Ö½ÚÎª²»¿É´ï´úÂë£¬Çë²Î¿¼ ²»¿É´ï´úÂë(ch395inc.h)
     @arg       µÚ2¸ö×Ö½ÚÎªip°üĞ­ÒéÀàĞÍ
     @arg       µÚ3-4×Ö½ÚÎª¶Ë¿ÚºÅ
     @arg       µÚ4-8×Ö½ÚÎªipµØÖ·
 * @retval      ÎŞ
 */
void ch395_cmd_get_unreachippt(uint8_t *list)
{
    uint8_t i;

    ch395_write_cmd(CMD08_GET_UNREACH_IPPORT);

    for (i = 0; i < 8; i++)
    {
        *list++ = ch395_read_data();
    }

    ch395_scs_hign;
}

/**
 * @brief       »ñÈ¡Ô¶¶ËµÄipºÍ¶Ë¿ÚµØÖ·£¬Ò»°ãÔÚtcp serverÄ£Ê½ÏÂÊ¹ÓÃ
 * @param       sockindex socketË÷Òı
 * @param       list ±£´æipºÍ¶Ë¿Ú
 * @retval      ÎŞ
 */
void ch395_cmd_get_remoteipp(uint8_t sockindex, uint8_t *list)
{
    uint8_t i;

    ch395_write_cmd(CMD06_GET_REMOT_IPP_SN);
    ch395_write_data(sockindex);

    for (i = 0; i < 6; i++)
    {
        *list++ = ch395_read_data();
    }

    ch395_scs_hign;
}

/**
 * @brief       ÉèÖÃsocket nµÄÄ¿µÄipµØÖ·
 * @param       sockindex socketË÷Òı
 * @param       ipaddr Ö¸ÏòipµØÖ·
 * @retval      ÎŞ
 */
void ch395_set_socket_desip(uint8_t sockindex, uint8_t *ipaddr)
{
    ch395_write_cmd(CMD50_SET_IP_ADDR_SN);
    ch395_write_data(sockindex);
    ch395_write_data(*ipaddr++);
    ch395_write_data(*ipaddr++);
    ch395_write_data(*ipaddr++);
    ch395_write_data(*ipaddr++);
    ch395_scs_hign;
}

/**
 * @brief       ÉèÖÃsocket µÄĞ­ÒéÀàĞÍ
 * @param       sockindex socketË÷Òı,prottype Ğ­ÒéÀàĞÍ
 * @param       Çë²Î¿¼ socketĞ­ÒéÀàĞÍ¶¨Òå(ch395inc.h)
 * @retval      ÎŞ
 */
void ch395_set_socket_prot_type(uint8_t sockindex, uint8_t prottype)
{
    ch395_write_cmd(CMD20_SET_PROTO_TYPE_SN);
    ch395_write_data(sockindex);
    ch395_write_data(prottype);
    ch395_scs_hign;
}

/**
 * @brief       ÉèÖÃsocket nµÄĞ­ÒéÀàĞÍ
 * @param       sockindex socketË÷Òı
 * @param       desprot 2×Ö½ÚÄ¿µÄ¶Ë¿Ú
 * @retval      ÎŞ
 */
void ch395_set_socket_desport(uint8_t sockindex, uint16_t desprot)
{
    ch395_write_cmd(CMD30_SET_DES_PORT_SN);
    ch395_write_data(sockindex);
    ch395_write_data((uint8_t)desprot);
    ch395_write_data((uint8_t)(desprot >> 8));
    ch395_scs_hign;
}


/**
 * @brief       ÉèÖÃsocket nµÄĞ­ÒéÀàĞÍ
 * @param       sockindex socketË÷Òı
 * @param       desprot 2×Ö½ÚÔ´¶Ë¿Ú
 * @retval      ÎŞ
 */
void ch395_set_socket_sourport(uint8_t sockindex, uint16_t surprot)
{
    ch395_write_cmd(CMD30_SET_SOUR_PORT_SN);
    ch395_write_data(sockindex);
    ch395_write_data((uint8_t)surprot);
    ch395_write_data((uint8_t)(surprot >> 8));
    ch395_scs_hign;
}

/**
 * @brief       ipÄ£Ê½ÏÂ£¬socket ip°üĞ­Òé×Ö¶Î
 * @param       sockindex socketË÷Òı
 * @param       prototype iprawÄ£Ê½1×Ö½ÚĞ­Òé×Ö¶Î
 * @retval      ÎŞ
 */
void ch395_set_socket_ipraw_proto(uint8_t sockindex, uint8_t prototype)
{
    ch395_write_cmd(CMD20_SET_IPRAW_PRO_SN);
    ch395_write_data(sockindex);
    ch395_write_data(prototype);
    ch395_scs_hign;
}

/**
 * @brief       ¿ªÆô/¹Ø±Õ ping
 * @param       senable :0 / 1, ¾ßÌåº¬ÒåÈçÏÂ:
 *   @arg       1:  ¿ªÆôping
 *   @arg       0:  ¹Ø±Õping
 * @retval      ÎŞ
 */
void ch395_enable_ping(uint8_t enable)
{
    ch395_write_cmd(CMD01_PING_ENABLE);
    ch395_write_data(enable);
    ch395_scs_hign;
}

/**
 * @brief       Ïò·¢ËÍ»º³åÇøĞ´Êı¾İ
 * @param       sockindex socketË÷Òı
 * @param       databuf  Êı¾İ»º³åÇø
 * @param       len   ³¤¶È
 * @retval      ÎŞ
 */
void ch395_send_data(uint8_t sockindex, uint8_t *databuf, uint16_t len)
{
    uint16_t i;

    ch395_write_cmd(CMD30_WRITE_SEND_BUF_SN);
    ch395_write_data((uint8_t)sockindex);
    ch395_write_data((uint8_t)len);
    ch395_write_data((uint8_t)(len >> 8));

    for (i = 0; i < len; i++)
    {
        ch395_write_data(*databuf++);
    }

    ch395_scs_hign;
}

/**
 * @brief       »ñÈ¡½ÓÊÕ»º³åÇø³¤¶È
 * @param       sockindex socketË÷Òı
 * @retval      ·µ»Ø½ÓÊÕ»º³åÇøÓĞĞ§³¤¶È
 */
uint16_t ch395_get_recv_length(uint8_t sockindex)
{
    uint16_t i;

    ch395_write_cmd(CMD12_GET_RECV_LEN_SN);
    ch395_write_data((uint8_t)sockindex);
    i = ch395_read_data();
    i = (uint16_t)(ch395_read_data() << 8) + i;
    ch395_scs_hign;
    return i;
}

/**
 * @brief       Çå³ı½ÓÊÕ»º³åÇø
 * @param       sockindex socketË÷Òı
 * @retval      ÎŞ
 */
void ch395_clear_recv_buf(uint8_t sockindex)
{
    ch395_write_cmd(CMD10_CLEAR_RECV_BUF_SN);
    ch395_write_data((uint8_t)sockindex);
    ch395_scs_hign;
}

/**
 * @brief       ¶ÁÈ¡½ÓÊÕ»º³åÇøÊı¾İ
 * @param       sockindex socketË÷Òı
 * @param       len  ³¤¶È
 * @param       pbuf  »º³åÇø
 * @retval      ÎŞ
 */
void ch395_get_recv_data(uint8_t sockindex, uint16_t len, uint8_t *pbuf)
{
    uint16_t i;

    if (!len)return;

    ch395_write_cmd(CMD30_READ_RECV_BUF_SN);
    ch395_write_data(sockindex);
    ch395_write_data((uint8_t)len);
    ch395_write_data((uint8_t)(len >> 8));
    delay_us(1);

    for (i = 0; i < len; i++)
    {
        *pbuf = ch395_read_data();
        pbuf++;
    }

    ch395_scs_hign;
}

/**
 * @brief       ÉèÖÃÖØÊÔ´ÎÊı
 * @param       count ÖØÊÔÖµ£¬×î´óÎª20´Î
 * @retval      ÎŞ
 */
void ch395_cmd_set_retry_count(uint8_t count)
{
    ch395_write_cmd(CMD10_SET_RETRAN_COUNT);
    ch395_write_data(count);
    ch395_scs_hign;
}

/**
 * @brief       ÉèÖÃÖØÊÔÖÜÆÚ
 * @param       period ÖØÊÔÖÜÆÚµ¥Î»ÎªºÁÃë£¬×î´ó1000ms
 * @retval      ÎŞ
 */
void ch395_cmd_set_retry_period(uint16_t period)
{
    ch395_write_cmd(CMD10_SET_RETRAN_COUNT);
    ch395_write_data((uint8_t)period);
    ch395_write_data((uint8_t)(period >> 8));
    ch395_scs_hign;
}

/**
 * @brief       »ñÈ¡socket
 * @param       sockindex socketË÷Òı
 * @retval      socket nµÄ×´Ì¬ĞÅÏ¢£¬µÚ1×Ö½ÚÎªsocket ´ò¿ª»òÕß¹Ø±Õ,µÚ2×Ö½ÚÎªtcp×´Ì¬
 */
void ch395_cmd_get_socket_status(uint8_t sockindex, uint8_t *status)
{
    ch395_write_cmd(CMD12_GET_SOCKET_STATUS_SN);
    ch395_write_data(sockindex);
    *status++ = ch395_read_data();
    *status++ = ch395_read_data();
    ch395_scs_hign;
}

/**
 * @brief       ´ò¿ªsocket£¬´ËÃüÁîĞèÒªµÈ´ıÖ´ĞĞ³É¹¦
 * @param       sockindex socketË÷Òı
 * @retval      ·µ»ØÖ´ĞĞ½á¹û
 */
uint8_t  ch395_open_socket(uint8_t sockindex)
{
    uint8_t i = 0;
    uint8_t s = 0;
    ch395_write_cmd(CMD1W_OPEN_SOCKET_SN);
    ch395_write_data(sockindex);
    ch395_scs_hign;

    while (1)
    {
        delay_ms(5);                          /* ÑÓÊ±²éÑ¯£¬½¨Òé2MSÒÔÉÏ */
        s = ch395_get_cmd_status();           /* ²»ÄÜ¹ıÓÚÆµ·±²éÑ¯ */

        if (s != CH395_ERR_BUSY)
        {
            break;        /* Èç¹ûCH395Ğ¾Æ¬·µ»ØÃ¦×´Ì¬ */
        }

        if (i++ > 200)
        {
            return CH395_ERR_UNKNOW; /* ³¬Ê±ÍË³ö */
        }
    }

    return s;
}

/**
 * @brief       ¹Ø±Õsocket£¬
 * @param       sockindex socketË÷Òı
 * @retval      ·µ»ØÖ´ĞĞ½á¹û
 */
uint8_t  ch395_close_socket(uint8_t sockindex)
{
    uint8_t i = 0;
    uint8_t s = 0;
    ch395_write_cmd(CMD1W_CLOSE_SOCKET_SN);
    ch395_write_data(sockindex);
    ch395_scs_hign;

    while (1)
    {
        delay_ms(5);                            /* ÑÓÊ±²éÑ¯£¬½¨Òé2MSÒÔÉÏ */
        s = ch395_get_cmd_status();             /* ²»ÄÜ¹ıÓÚÆµ·±²éÑ¯ */

        if (s != CH395_ERR_BUSY)
        {
            break;          /* Èç¹ûCH395Ğ¾Æ¬·µ»ØÃ¦×´Ì¬ */
        }

        if (i++ > 200)
        {
            return CH395_ERR_UNKNOW;  /* ³¬Ê±ÍË³ö */
        }
    }

    return s;
}

/**
 * @brief       tcpÁ¬½Ó£¬½öÔÚtcpÄ£Ê½ÏÂÓĞĞ§£¬´ËÃüÁîĞèÒªµÈ´ıÖ´ĞĞ³É¹¦
 * @param       sockindex socketË÷Òı
 * @retval      ·µ»ØÖ´ĞĞ½á¹û
 */
uint8_t ch395_tcp_connect(uint8_t sockindex)
{
    uint8_t i = 0;
    uint8_t s = 0;
    ch395_write_cmd(CMD1W_TCP_CONNECT_SN);
    ch395_write_data(sockindex);
    ch395_scs_hign;

    while (1)
    {
        delay_ms(5);                            /* ÑÓÊ±²éÑ¯£¬½¨Òé2MSÒÔÉÏ */
        s = ch395_get_cmd_status();             /* ²»ÄÜ¹ıÓÚÆµ·±²éÑ¯ */

        if (s != CH395_ERR_BUSY)
        {
            break;          /* Èç¹ûCH395Ğ¾Æ¬·µ»ØÃ¦×´Ì¬ */
        }

        if (i++ > 200)
        {
            return CH395_ERR_UNKNOW;  /* ³¬Ê±ÍË³ö */
        }
    }

    return s;
}

/**
 * @brief       tcp¼àÌı£¬½öÔÚtcpÄ£Ê½ÏÂÓĞĞ§£¬´ËÃüÁîĞèÒªµÈ´ıÖ´ĞĞ³É¹¦
 * @param       sockindex socketË÷Òı
 * @retval      ·µ»ØÖ´ĞĞ½á¹û
 */
uint8_t ch395_tcp_listen(uint8_t sockindex)
{
    uint8_t i = 0;
    uint8_t s = 0;
    ch395_write_cmd(CMD1W_TCP_LISTEN_SN);
    ch395_write_data(sockindex);
    ch395_scs_hign;

    while (1)
    {
        delay_ms(5);                           /* ÑÓÊ±²éÑ¯£¬½¨Òé2MSÒÔÉÏ */
        s = ch395_get_cmd_status();            /* ²»ÄÜ¹ıÓÚÆµ·±²éÑ¯ */

        if (s != CH395_ERR_BUSY)
        {
            break;         /* Èç¹ûCH395Ğ¾Æ¬·µ»ØÃ¦×´Ì¬ */
        }

        if (i++ > 200)
        {
            return CH395_ERR_UNKNOW; /* ³¬Ê±ÍË³ö */
        }
    }

    return s;
}

/**
 * @brief       tcp¶Ï¿ª£¬½öÔÚtcpÄ£Ê½ÏÂÓĞĞ§£¬´ËÃüÁîĞèÒªµÈ´ıÖ´ĞĞ³É¹¦
 * @param       sockindex socketË÷Òı
 * @retval      ÎŞ
 */
uint8_t ch395_tcp_disconnect(uint8_t sockindex)
{
    uint8_t i = 0;
    uint8_t s = 0;
    ch395_write_cmd(CMD1W_TCP_DISNCONNECT_SN);
    ch395_write_data(sockindex);
    ch395_scs_hign;

    while (1)
    {
        delay_ms(5);                           /* ÑÓÊ±²éÑ¯£¬½¨Òé2MSÒÔÉÏ */
        s = ch395_get_cmd_status();               /* ²»ÄÜ¹ıÓÚÆµ·±²éÑ¯ */

        if (s != CH395_ERR_BUSY)
        {
            break;         /* Èç¹ûCH395Ğ¾Æ¬·µ»ØÃ¦×´Ì¬ */
        }

        if (i++ > 200)
        {
            return CH395_ERR_UNKNOW; /* ³¬Ê±ÍË³ö */
        }
    }

    return s;
}

/**
 * @brief       »ñÈ¡socket nµÄÖĞ¶Ï×´Ì¬
 * @param       sockindex   socketË÷Òı
 * @retval      ÖĞ¶Ï×´Ì¬
 */
uint8_t ch395_get_socket_int(uint8_t sockindex)
{
    uint8_t intstatus;
    ch395_write_cmd(CMD11_GET_INT_STATUS_SN);
    ch395_write_data(sockindex);
    delay_us(2);
    intstatus = ch395_read_data();
    ch395_scs_hign;
    return intstatus;
}

/**
 * @brief       ¶Ô¶à²¥µØÖ·½øĞĞcrcÔËËã£¬²¢È¡¸ß6Î»¡£
 * @param       mac_addr   macµØÖ·
 * @retval      ·µ»Øcrc32µÄ¸ß6Î»
 */
uint8_t ch395_crcret_6bit(uint8_t *mac_addr)
{
    signed long perbyte;
    signed long perbit;
    const uint32_t poly = 0x04c11db7;
    uint32_t crc_value = 0xffffffff;
    uint8_t c;

    for ( perbyte = 0; perbyte < 6; perbyte ++ )
    {
        c = *(mac_addr++);

        for ( perbit = 0; perbit < 8; perbit++ )
        {
            crc_value = (crc_value << 1) ^ ((((crc_value >> 31)^c) & 0x01) ? poly : 0);
            c >>= 1;
        }
    }

    crc_value = crc_value >> 26;
    return ((uint8_t)crc_value);
}

/**
 * @brief       Æô¶¯/Í£Ö¹dhcp
 * @param       flag:0 / 1, ¾ßÌåº¬ÒåÈçÏÂ:
 *   @arg       1:Æô¶¯dhcp
 *   @arg       0£ºÍ£Ö¹dhcp
 * @retval      Ö´ĞĞ×´Ì¬
 */
uint8_t  ch395_dhcp_enable(uint8_t flag)
{
    uint8_t i = 0;
    uint8_t s;
    ch395_write_cmd(CMD10_DHCP_ENABLE);
    ch395_write_data(flag);
    ch395_scs_hign;

    while (1)
    {
        delay_ms(20);
        s = ch395_get_cmd_status();            /* ²»ÄÜ¹ıÓÚÆµ·±²éÑ¯ */

        if (s != CH395_ERR_BUSY)
        {
            break;         /* Èç¹ûch395Ğ¾Æ¬·µ»ØÃ¦×´Ì¬ */
        }

        if (i++ > 200)
        {
            return CH395_ERR_UNKNOW; /* ³¬Ê±ÍË³ö */
        }
    }

    return s;
}

/**
 * @brief       »ñÈ¡dhcp×´Ì¬
 * @param       ÎŞ
 * @retval      dhcp×´Ì¬£¬0Îª³É¹¦£¬ÆäËûÖµ±íÊ¾´íÎó
 */
uint8_t ch395_get_dhcp_status(void)
{
    uint8_t status;
    ch395_write_cmd(CMD01_GET_DHCP_STATUS);
    status = ch395_read_data();
    ch395_scs_hign;
    return status;
}

/**
 * @brief       »ñÈ¡ip£¬×ÓÍøÑÚÂëºÍÍø¹ØµØÖ·
 * @param       sockindex socketË÷Òı
 * @retval      12¸ö×Ö½ÚµÄip,×ÓÍøÑÚÂëºÍÍø¹ØµØÖ·
 */
void ch395_get_ipinf(uint8_t *addr)
{
    uint8_t i;
    ch395_write_cmd(CMD014_GET_IP_INF);

    for (i = 0; i < 20; i++)
    {
        *addr++ = ch395_read_data();
    }

    ch395_scs_hign;
}

/**
 * @brief       Ğ´gpio¼Ä´æÆ÷
 * @param       regadd   ¼Ä´æÆ÷µØÖ·
 * @param       regval   ¼Ä´æÆ÷Öµ
 * @retval      ÎŞ
 */
void ch395_write_gpio_addr(uint8_t regadd, uint8_t regval)
{
    ch395_write_cmd(CMD20_WRITE_GPIO_REG);
    ch395_write_data(regadd);
    ch395_write_data(regval);
}

/**
 * @brief       ¶Ágpio¼Ä´æÆ÷
 * @param       regadd   ¼Ä´æÆ÷µØÖ·
 * @retval      ¼Ä´æÆ÷µÄÖµ
 */
uint8_t ch395_read_gpio_addr(uint8_t regadd)
{
    uint8_t i;
    ch395_write_cmd(CMD10_READ_GPIO_REG);
    ch395_write_data(regadd);
    delay_ms(1);
    i = ch395_read_data();
    return i;
}

/**
 * @brief       ²Á³ıeeprom
 * @param       ÎŞ
 * @retval      Ö´ĞĞ×´Ì¬
 */
uint8_t ch395_eeprom_erase(void)
{
    uint8_t i;
    ch395_write_cmd(CMD00_EEPROM_ERASE);

    while (1)
    {
        delay_ms(20);
        i = ch395_get_cmd_status();

        if (i == CH395_ERR_BUSY)
        {
            continue;
        }

        break;
    }

    return i;
}

/**
 * @brief       Ğ´eeprom
 * @param       eepaddr  eepromµØÖ·
 * @param       buf      »º³åÇøµØÖ·
 * @param       len      ³¤¶È
 * @retval      ÎŞ
 */
uint8_t ch395_eeprom_write(uint16_t eepaddr, uint8_t *buf, uint8_t len)
{
    uint8_t i;
    ch395_write_cmd(CMD30_EEPROM_WRITE);
    ch395_write_data((uint8_t)(eepaddr));
    ch395_write_data((uint8_t)(eepaddr >> 8));
    ch395_write_data(len);

    while (len--)ch395_write_data(*buf++);

    while (1)
    {
        delay_ms(20);
        i = ch395_get_cmd_status();

        if (i == CH395_ERR_BUSY)
        {
            continue;
        }

        break;
    }

    return i;
}

/**
 * @brief       Ğ´eeprom
 * @param       eepaddr  eepromµØÖ·
 * @param       buf      »º³åÇøµØÖ·
 * @param       len      ³¤¶È
 * @retval      ÎŞ
 */
void ch395_eeprom_read(uint16_t eepaddr, uint8_t *buf, uint8_t len)
{
    ch395_write_cmd(CMD30_EEPROM_READ);
    ch395_write_data((uint8_t)(eepaddr));
    ch395_write_data((uint8_t)(eepaddr >> 8));
    ch395_write_data(len);
    delay_ms(1);

    while (len--)
    {
        *buf++ = ch395_read_data();
    }
}

/**
 * @brief       ÉèÖÃtcp mssÖµ
 * @param       tcpmss
 * @retval      ÎŞ
 */
void ch395_set_tcpmss(uint16_t tcpmss)
{
    ch395_write_cmd(CMD20_SET_TCP_MSS);
    ch395_write_data((uint8_t)(tcpmss));
    ch395_write_data((uint8_t)(tcpmss >> 8));
}

/**
 * @brief       ÉèÖÃsocket½ÓÊÕ»º³åÇø
 * @param       sockindex  socketË÷Òı,Ö·,blknum
 * @param       startblk   ÆğÊ¼µØ
 * @param       µ¥Î»»º³åÇø¸öÊı £¬µ¥Î»Îª512×Ö½Ú
 * @retval      ÎŞ
 */
void ch395_set_socket_recv_buf(uint8_t sockindex, uint8_t startblk, uint8_t blknum)
{
    ch395_write_cmd(CMD30_SET_RECV_BUF);
    ch395_write_data(sockindex);
    ch395_write_data(startblk);
    ch395_write_data(blknum);
}

/**
 * @brief       ÉèÖÃsocket·¢ËÍ»º³åÇø
 * @param       sockindex  socketË÷Òı
 * @param       startblk   ÆğÊ¼µØÖ·
 * @param       blknum     µ¥Î»»º³åÇø¸öÊı
 * @retval      ÎŞ
 */
void ch395_set_socket_send_buf(uint8_t sockindex, uint8_t startblk, uint8_t blknum)
{
    ch395_write_cmd(CMD30_SET_SEND_BUF);
    ch395_write_data(sockindex);
    ch395_write_data(startblk);
    ch395_write_data(blknum);
}

/**
 * @brief       udpÏòÖ¸¶¨µÄipºÍ¶Ë¿Ú·¢ËÍÊı¾İ
 * @param       buf     : ·¢ËÍÊı¾İ»º³åÇø
 * @param       len     : ·¢ËÍÊı¾İ³¤¶È
 * @param       ip      : Ä¿±êip
 * @param       port    : Ä¿±ê¶Ë¿Ú
 * @param       sockeid : socketË÷ÒıÖµ
 * @retval      ÎŞ
 */
void ch395_udp_send_data(uint8_t *buf, uint32_t len, uint8_t *ip, uint16_t port, uint8_t sockindex)
{
    ch395_set_socket_desip(sockindex, ip);   /* ÉèÖÃsocket 0Ä¿±êIPµØÖ· */
    ch395_set_socket_desport(sockindex, port);
    ch395_send_data(sockindex, buf, len);
}

/**
 * @brief       ÉèÖÃch395Æô¶¯²ÎÊı
 * @param       mdata ÉèÖÃµÄ²ÎÊı
 * @retval      ÎŞ
 */
void ch395_set_start_para(uint32_t mdata)
{
    ch395_write_cmd(CMD40_SET_FUN_PARA);
    ch395_write_data((uint8_t)mdata);
    ch395_write_data((uint8_t)((uint16_t)mdata >> 8));
    ch395_write_data((uint8_t)(mdata >> 16));
    ch395_write_data((uint8_t)(mdata >> 24));
}

/**
 * @brief       »ñÈ¡È«¾ÖÖĞ¶Ï×´Ì¬£¬ÊÕµ½´ËÃüÁîch395×Ô¶¯È¡ÏûÖĞ¶Ï,0x44¼°ÒÔÉÏ°æ±¾Ê¹ÓÃ
 * @param       ÎŞ
 * @retval      ·µ»Øµ±Ç°µÄÈ«¾ÖÖĞ¶Ï×´Ì¬
 */
uint16_t ch395_cmd_get_glob_int_status_all(void)
{
    uint16_t init_status;
    ch395_write_cmd(CMD02_GET_GLOB_INT_STATUS_ALL);
    delay_us(2);
    init_status = ch395_read_data();
    init_status = (uint16_t)(ch395_read_data() << 8) + init_status;
    ch395_scs_hign;
    return  init_status;
}

/**
 * @brief       ÉèÖÃkeepalive¹¦ÄÜ
 * @param       sockindex socketºÅ
 * @param       cmd 0£º¹Ø±Õ 1£º¿ªÆô
 * @retval      ÎŞ
 */
void ch395_set_keeplive(uint8_t sockindex, uint8_t cmd)
{
    ch395_write_cmd(CMD20_SET_KEEP_LIVE_SN);
    ch395_write_data(sockindex);
    ch395_write_data(cmd);
}

/**
 * @brief       ÉèÖÃkeepaliveÖØÊÔ´ÎÊı
 * @param       cnt ÖØÊÔ´ÎÊı£¨£©
 * @retval      ÎŞ
 */
void ch395_keeplive_cnt(uint8_t cnt)
{
    ch395_write_cmd(CMD10_SET_KEEP_LIVE_CNT);
    ch395_write_data(cnt);
}

/**
 * @brief       ÉèÖÃkeeplive¿ÕÏĞ
 * @param       idle ¿ÕÏĞÊ±¼ä£¨µ¥Î»£ºms£©
 * @retval      ÎŞ
 */
void ch395_keeplive_idle(uint32_t idle)
{
    ch395_write_cmd(CMD40_SET_KEEP_LIVE_IDLE);
    ch395_write_data((uint8_t)idle);
    ch395_write_data((uint8_t)((uint16_t)idle >> 8));
    ch395_write_data((uint8_t)(idle >> 16));
    ch395_write_data((uint8_t)(idle >> 24));
}

/**
 * @brief       ÉèÖÃkeeplive¼ä¸ôÊ±¼ä
 * @param       intvl ¼ä¸ôÊ±¼ä£¨µ¥Î»£ºms£©
 * @retval      ÎŞ
 */
void ch395_keeplive_intvl(uint32_t intvl)
{
    ch395_write_cmd(CMD40_SET_KEEP_LIVE_INTVL);
    ch395_write_data((uint8_t)intvl);
    ch395_write_data((uint8_t)((uint16_t)intvl >> 8));
    ch395_write_data((uint8_t)(intvl >> 16));
    ch395_write_data((uint8_t)(intvl >> 24));
}

/**
 * @brief       ÉèÖÃttl
 * @param       ssockindex socketºÅ
 * @param       ttlnum:ttlÊı
 * @retval      ÎŞ
 */
void ch395_setttl_num(uint8_t sockindex, uint8_t ttlnum)
{
    ch395_write_cmd(CMD20_SET_TTL);
    ch395_write_data(sockindex);
    ch395_write_data(ttlnum);
}
