/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision$
 * $Date$
 *
 * Purpose : RTK switch high-level API for RTL8367/RTL8367D
 * Feature : Here is a list of all functions and variables in Port module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal/rtl8367d/dal_rtl8367d_port.h>
#include <dal/rtl8367d/rtl8367d_asicdrv.h>
#include <string.h>

#define RTL8367D_PORT_SDS_MODE_DISABLE      0x1f
#define RTL8367D_PORT_SDS_MODE_SGMII        0x2
#define RTL8367D_PORT_SDS_MODE_HSGMII       0x12
#define RTL8367D_PORT_SDS_MODE_1000X        0x4
#define RTL8367D_PORT_SDS_MODE_100FX        0x5
#define RTL8367D_PORT_SDS_MODE_1000X_100FX  0x7

#define RTL8367D_EXT_PORT_SPEED_10M         0x0
#define RTL8367D_EXT_PORT_SPEED_100M        0x1
#define RTL8367D_EXT_PORT_SPEED_1000M       0x2
#define RTL8367D_EXT_PORT_SPEED_500M        0x3
#define RTL8367D_EXT_PORT_SPEED_2500M       0x5


typedef struct  rtl8367d_port_ability_s{
    rtk_uint16 forcemode;
    rtk_uint16 mstfault;
    rtk_uint16 mstmode;
    rtk_uint16 nway;
    rtk_uint16 txpause;
    rtk_uint16 rxpause;
    rtk_uint16 link;
    rtk_uint16 duplex;
    rtk_uint16 speed;
}rtl8367d_port_ability_t;

static rtk_api_ret_t _dal_rtl8367d_port_FiberModeAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      regData;

    /* Check Combo port or not */
    RTK_CHK_PORT_IS_COMBO(port);

    /* Flow Control */
    if ((retVal = rtl8367d_getAsicReg(RTL8367D_REG_FIB_CFG04, &regData)) != RT_ERR_OK)
        return retVal;

    if (pAbility->AsyFC == 1)
        regData |= (0x0001 << 8);
    else
        regData &= ~(0x0001 << 8);

    if (pAbility->FC == 1)
        regData |= (0x0001 << 7);
    else
        regData &= ~(0x0001 << 7);

    if ((retVal = rtl8367d_setAsicReg(RTL8367D_REG_FIB_CFG04, regData)) != RT_ERR_OK)
        return retVal;

    /* Speed ability */
    if( (pAbility->Full_1000 == 1) && (pAbility->Full_100 == 1) && (pAbility->AutoNegotiation == 1) )
    {
        if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS_MISC, RTL8367D_CFG_SDS_MODE_MASK, RTL8367D_PORT_SDS_MODE_DISABLE)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS_MISC, RTL8367D_CFG_SDS_MODE_MASK, RTL8367D_PORT_SDS_MODE_1000X_100FX)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367d_setAsicReg(RTL8367D_REG_FIB_CFG00, 0x1340)) != RT_ERR_OK)
            return retVal;
    }
    else if(pAbility->Full_1000 == 1)
    {
        if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS_MISC, RTL8367D_CFG_SDS_MODE_MASK, RTL8367D_PORT_SDS_MODE_DISABLE)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS_MISC, RTL8367D_CFG_SDS_MODE_MASK, RTL8367D_PORT_SDS_MODE_1000X)) != RT_ERR_OK)
            return retVal;

        if(pAbility->AutoNegotiation == 1)
        {
            if ((retVal = rtl8367d_setAsicReg(RTL8367D_REG_FIB_CFG00, 0x1140)) != RT_ERR_OK)
                return retVal;
        }
        else
        {
            if ((retVal = rtl8367d_setAsicReg(RTL8367D_REG_FIB_CFG00, 0x0140)) != RT_ERR_OK)
                return retVal;
        }
    }
    else if(pAbility->Full_100 == 1)
    {
        if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS_MISC, RTL8367D_CFG_SDS_MODE_MASK, RTL8367D_PORT_SDS_MODE_DISABLE)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS_MISC, RTL8367D_CFG_SDS_MODE_MASK, RTL8367D_PORT_SDS_MODE_100FX)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367d_setAsicReg(RTL8367D_REG_FIB_CFG00, 0x2100)) != RT_ERR_OK)
            return retVal;
    }

    /* Restart N-way */
    if ((retVal = rtl8367d_getAsicReg(RTL8367D_REG_FIB_CFG00, &regData)) != RT_ERR_OK)
        return retVal;

    regData |= (0x0001 << 9);

    if ((retVal = rtl8367d_setAsicReg(RTL8367D_REG_FIB_CFG00, regData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

static rtk_api_ret_t _dal_rtl8367d_port_FiberModeAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      regData;
    rtk_uint32      sdsMode;

    /* Check Combo port or not */
    RTK_CHK_PORT_IS_COMBO(port);

    if (pAbility == NULL)
        return RT_ERR_NULL_POINTER;

    memset(pAbility, 0x00, sizeof(rtk_port_phy_ability_t));

    if ((retVal = rtl8367d_getAsicReg(RTL8367D_REG_FIB_CFG04, &regData)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367d_getAsicRegBits(RTL8367D_REG_SDS_MISC, RTL8367D_CFG_SDS_MODE_MASK, &sdsMode)) != RT_ERR_OK)
            return retVal;

    if(regData & (0x0001 << 8))
        pAbility->AsyFC = 1;

    if(regData & (0x0001 << 7))
        pAbility->FC = 1;

    switch (sdsMode)
    {
        case RTL8367D_PORT_SDS_MODE_1000X_100FX:
            pAbility->AutoNegotiation = 1;
            pAbility->Full_1000 = 1;
            pAbility->Full_100 = 1;
            break;
        case RTL8367D_PORT_SDS_MODE_1000X:
            pAbility->Full_1000 = 1;

            if ((retVal = rtl8367d_getAsicReg(RTL8367D_REG_FIB_CFG00, &regData)) != RT_ERR_OK)
                return retVal;

            if(regData & 0x1000)
                pAbility->AutoNegotiation = 1;
            else
                pAbility->AutoNegotiation = 0;

            break;
        case RTL8367D_PORT_SDS_MODE_100FX:
            pAbility->Full_100 = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}

static rtk_api_ret_t _dal_rtl8367d_setAsicPHYOCPReg(rtk_uint32 phyNo, rtk_uint32 ocpAddr, rtk_uint32 ocpData )
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint32 busyFlag, checkCounter;
    rtk_uint32 ocpAddrPrefix, ocpAddr9_6, ocpAddr5_1;

    /*Check internal phy access busy or not*/
    retVal = rtl8367d_getAsicReg(RTL8367D_REG_INDRECT_ACCESS_STATUS, &busyFlag);
    if(retVal != RT_ERR_OK)
        return retVal;

    if(busyFlag)
        return RT_ERR_BUSYWAIT_TIMEOUT;

    if ((retVal = rtl8367d_getAsicReg(RTL8367D_REG_FPGA_VER_CEN, &regData)) != RT_ERR_OK)
        return retVal;

    /* OCP prefix */
    if (regData == 0)
        ocpAddrPrefix = ((ocpAddr & 0xFC00) >> 10); /* ASIC */
    else
        ocpAddrPrefix = 0; /* FPGA */

    if((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_GPHY_OCP_MSB_0, RTL8367D_CFG_CPU_OCPADR_MSB_MASK, ocpAddrPrefix)) != RT_ERR_OK)
        return retVal;

    /*prepare access data*/
    retVal = rtl8367d_setAsicReg(RTL8367D_REG_INDRECT_ACCESS_WRITE_DATA, ocpData);
    if(retVal != RT_ERR_OK)
        return retVal;

    /*prepare access address*/
    ocpAddr9_6 = ((ocpAddr >> 6) & 0x000F);
    ocpAddr5_1 = ((ocpAddr >> 1) & 0x001F);
    regData = RTL8367D_PHY_BASE | (ocpAddr9_6 << 8) | (phyNo << RTL8367D_PHY_OFFSET) | ocpAddr5_1;
    retVal = rtl8367d_setAsicReg(RTL8367D_REG_INDRECT_ACCESS_ADDRESS, regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    /*Set WRITE Command*/
    retVal = rtl8367d_setAsicReg(RTL8367D_REG_INDRECT_ACCESS_CTRL, RTL8367D_CMD_MASK | RTL8367D_RW_MASK);

    checkCounter = 100;
    while(checkCounter)
    {
        retVal = rtl8367d_getAsicReg(RTL8367D_REG_INDRECT_ACCESS_STATUS, &busyFlag);
        if((retVal != RT_ERR_OK) || busyFlag)
        {
            checkCounter --;
            if(0 == checkCounter)
                return RT_ERR_BUSYWAIT_TIMEOUT;
        }
        else
        {
            checkCounter = 0;
        }
    }

    return retVal;
}

static rtk_api_ret_t _dal_rtl8367d_getAsicPHYOCPReg(rtk_uint32 phyNo, rtk_uint32 ocpAddr, rtk_uint32 *pRegData )
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint32 busyFlag,checkCounter;
    rtk_uint32 ocpAddrPrefix, ocpAddr9_6, ocpAddr5_1;

    /*Check internal phy access busy or not*/
    retVal = rtl8367d_getAsicReg(RTL8367D_REG_INDRECT_ACCESS_STATUS, &busyFlag);
    if(retVal != RT_ERR_OK)
        return retVal;

    if(busyFlag)
        return RT_ERR_BUSYWAIT_TIMEOUT;

    if ((retVal = rtl8367d_getAsicReg(RTL8367D_REG_FPGA_VER_CEN, &regData)) != RT_ERR_OK)
        return retVal;

    /* OCP prefix */
    if (regData == 0)
        ocpAddrPrefix = ((ocpAddr & 0xFC00) >> 10); /* ASIC */
    else
        ocpAddrPrefix = 0; /* FPGA */

    if((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_GPHY_OCP_MSB_0, RTL8367D_CFG_CPU_OCPADR_MSB_MASK, ocpAddrPrefix)) != RT_ERR_OK)
        return retVal;

    /*prepare access address*/
    ocpAddr9_6 = ((ocpAddr >> 6) & 0x000F);
    ocpAddr5_1 = ((ocpAddr >> 1) & 0x001F);
    regData = RTL8367D_PHY_BASE | (ocpAddr9_6 << 8) | (phyNo << RTL8367D_PHY_OFFSET) | ocpAddr5_1;
    retVal = rtl8367d_setAsicReg(RTL8367D_REG_INDRECT_ACCESS_ADDRESS, regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    /*Set READ Command*/
    retVal = rtl8367d_setAsicReg(RTL8367D_REG_INDRECT_ACCESS_CTRL, RTL8367D_CMD_MASK );
    if(retVal != RT_ERR_OK)
        return retVal;

    checkCounter = 100;
    while(checkCounter)
    {
        retVal = rtl8367d_getAsicReg(RTL8367D_REG_INDRECT_ACCESS_STATUS, &busyFlag);
        if((retVal != RT_ERR_OK) || busyFlag)
        {
            checkCounter --;
            if(0 == checkCounter)
                return RT_ERR_FAILED;
        }
        else
        {
            checkCounter = 0;
        }
    }

    /*get PHY register*/
    retVal = rtl8367d_getAsicReg(RTL8367D_REG_INDRECT_ACCESS_READ_DATA, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pRegData = regData;
    return RT_ERR_OK;
}

static rtk_api_ret_t _dal_rtl8367d_setAsicPHYReg(rtk_uint32 phyNo, rtk_uint32 phyAddr, rtk_uint32 phyData )
{
    rtk_uint32 ocp_addr;

    if(phyAddr > RTL8367D_PHY_REGNOMAX)
        return RT_ERR_PHY_REG_ID;

    ocp_addr = 0xa400 + phyAddr*2;

    return _dal_rtl8367d_setAsicPHYOCPReg(phyNo, ocp_addr, phyData);
}

static rtk_api_ret_t _dal_rtl8367d_getAsicPHYReg(rtk_uint32 phyNo, rtk_uint32 phyAddr, rtk_uint32 *pRegData )
{
    rtk_uint32 ocp_addr;

    if(phyAddr > RTL8367D_PHY_REGNOMAX)
        return RT_ERR_PHY_REG_ID;

    ocp_addr = 0xa400 + phyAddr*2;

    return _dal_rtl8367d_getAsicPHYOCPReg(phyNo, ocp_addr, pRegData);
}

static rtk_api_ret_t _dal_rtl8367d_setAsicPortForceLinkExt(rtk_uint32 id, rtl8367d_port_ability_t *pPortAbility)
{
    rtk_api_ret_t retVal;
    rtk_uint32 reg_data = 0;
    rtk_uint32 reg_data2;

    /* Invalid input parameter */
    if ((id >= 3) || (id == 0))
        return RT_ERR_OUT_OF_RANGE;

    reg_data |= ((pPortAbility->speed & 0x0C) >> 2) << 12;
    reg_data |= pPortAbility->mstfault << 9;
    reg_data |= pPortAbility->mstmode << 8;
    reg_data |= pPortAbility->nway << 7;
    reg_data |= pPortAbility->txpause << 6;
    reg_data |= pPortAbility->rxpause << 5;
    reg_data |= pPortAbility->link << 4;
    reg_data |= pPortAbility->duplex << 2;
    reg_data |= pPortAbility->speed & 0x03;

    if(pPortAbility->forcemode)
        reg_data2 = 0xFFFF;
    else
        reg_data2 = 0;

    if((retVal = rtl8367d_setAsicReg(RTL8367D_REG_MAC5_FORCE_SELECT + id, reg_data)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367d_setAsicReg(RTL8367D_REG_MAC5_FORCE_SELECT_EN + id, reg_data2)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

static rtk_api_ret_t _dal_rtl8367d_getAsicPortForceLinkExt(rtk_uint32 id, rtl8367d_port_ability_t *pPortAbility)
{
    rtk_uint32  reg_data;
    rtk_uint32  reg_data2;
    rtk_api_ret_t  retVal;

    if ((id >= 3) || (id == 0))
        return RT_ERR_OUT_OF_RANGE;

    if((retVal = rtl8367d_getAsicReg(RTL8367D_REG_MAC5_FORCE_SELECT + id, &reg_data)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367d_getAsicReg(RTL8367D_REG_MAC5_FORCE_SELECT_EN + id, &reg_data2)) != RT_ERR_OK)
        return retVal;


    pPortAbility->forcemode = (reg_data2 == 0) ? 0 : 1;
    pPortAbility->mstfault  = (reg_data >> 9) & 0x0001;
    pPortAbility->mstmode   = (reg_data >> 8) & 0x0001;
    pPortAbility->nway      = (reg_data >> 7) & 0x0001;
    pPortAbility->txpause   = (reg_data >> 6) & 0x0001;
    pPortAbility->rxpause   = (reg_data >> 5) & 0x0001;
    pPortAbility->link      = (reg_data >> 4) & 0x0001;
    pPortAbility->duplex    = (reg_data >> 2) & 0x0001;
    pPortAbility->speed     = (reg_data & 0x0003) | (((reg_data & 0x3000) >> 12) << 2);

    return RT_ERR_OK;
}

static rtk_api_ret_t _dal_rtl8367d_setAsicPortExtMode(rtk_uint32 id, rtk_uint32 mode)
{
    rtk_api_ret_t  retVal;
    rtk_uint32     mux;
    rtk_uint32     i;
    rtk_uint32     HSGCfg[][2] = { {0x0421, 0x8E11}, {0x0423, 0x1020}, {0x0425, 0x5242}, {0x0427, 0x4E0C}, {0x0428, 0xAA00}, {0x0504, 0x051B}, {0x0000, 0x1400}, {0x0000, 0x1403} };
    rtk_uint32     SGCfg[][2] = {  {0x0423, 0x1020}, {0x0425, 0x5242}, {0x0427, 0x4E0C}, {0x0428, 0xAA00}, {0x0484, 0x031B}, {0x0000, 0x1400}, {0x0000, 0x1403} };

    if ((id >= 3) || (id == 0))
        return RT_ERR_OUT_OF_RANGE;

    if( (mode == MODE_EXT_TMII_MAC) || (mode == MODE_EXT_TMII_PHY) )
    {
        if( (retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_BYPASS_LINE_RATE, id, 1)) != RT_ERR_OK)
            return retVal;
    }
    else
    {
        if( (retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_BYPASS_LINE_RATE, id, 0)) != RT_ERR_OK)
            return retVal;
    }

    if (id == 1)
    {
        if (mode == MODE_EXT_DISABLE)
        {
            if((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_DIGITAL_INTERFACE_SELECT, RTL8367D_SELECT_GMII_1_MASK, 0)) != RT_ERR_OK)
                return retVal;

            if((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS_MISC, RTL8367D_CFG_SDS_MODE_MASK, RTL8367D_PORT_SDS_MODE_DISABLE)) != RT_ERR_OK)
                return retVal;

            if((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_SDS_MISC, RTL8367D_MAC6_SEL_SDS0_OFFSET, 0)) != RT_ERR_OK)
                return retVal;

            if((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_FIBER_CFG_2, RTL8367D_SDS_RX_DISABLE_MASK, 0)) != RT_ERR_OK)
                return retVal;
        }
        else if ((mode == MODE_EXT_SGMII) || (mode == MODE_EXT_HSGMII) || (mode == MODE_EXT_1000X_100FX) || (mode == MODE_EXT_1000X) || (mode == MODE_EXT_100FX))
        {
            if((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_DIGITAL_INTERFACE_SELECT, RTL8367D_SELECT_GMII_1_MASK, 0x0)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS_MISC, RTL8367D_CFG_SDS_MODE_MASK, RTL8367D_PORT_SDS_MODE_DISABLE)) != RT_ERR_OK)
                return retVal;

            if((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_SDS_MISC, RTL8367D_MAC6_SEL_SDS0_OFFSET, 1)) != RT_ERR_OK)
                return retVal;

            if((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_SDS_MISC, RTL8367D_PA12PC_EN_S0_OFFSET, 1)) != RT_ERR_OK)
                return retVal;

            if((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_SDS_MISC, RTL8367D_PA33PC_EN_S0_OFFSET, 1)) != RT_ERR_OK)
                return retVal;

            if((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_FIBER_CFG_2, RTL8367D_SDS_RX_DISABLE_MASK, 0x1)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_UTP_FIB_DET, RTL8367D_UTP_LINK_TIMER_MASK, 0x3)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_UTP_FIB_DET, RTL8367D_UTP_SDET_TIMER_MASK, 0x2)) != RT_ERR_OK)
                return retVal;

            switch (mode)
            {
                case MODE_EXT_SGMII:
                    for(i = 0; i < sizeof(SGCfg) / (sizeof(rtk_uint32) * 2); i++)
                    {
                        if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_DATA, SGCfg[i][1])) != RT_ERR_OK)
                            return retVal;

                        if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_ADR, SGCfg[i][0])) != RT_ERR_OK)
                            return retVal;

                        if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_CMD, 0x00CD)) != RT_ERR_OK)
                            return retVal;
                    }

                    if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS_MISC, RTL8367D_CFG_SDS_MODE_MASK, RTL8367D_PORT_SDS_MODE_SGMII)) != RT_ERR_OK)
                        return retVal;
                    break;
                case MODE_EXT_HSGMII:
                    for(i = 0; i < sizeof(HSGCfg) / (sizeof(rtk_uint32) * 2); i++)
                    {
                        if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_DATA, HSGCfg[i][1])) != RT_ERR_OK)
                            return retVal;

                        if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_ADR, HSGCfg[i][0])) != RT_ERR_OK)
                            return retVal;

                        if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_CMD, 0x00CD)) != RT_ERR_OK)
                            return retVal;
                    }

                    if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS_MISC, RTL8367D_CFG_SDS_MODE_MASK, RTL8367D_PORT_SDS_MODE_HSGMII)) != RT_ERR_OK)
                        return retVal;
                    break;
                case MODE_EXT_1000X_100FX:
                    if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_DATA, 0x1140)) != RT_ERR_OK)
                        return retVal;

                    if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_ADR, 0x0040)) != RT_ERR_OK)
                        return retVal;

                    if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_CMD, 0x00CD)) != RT_ERR_OK)
                        return retVal;

                    if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS_MISC, RTL8367D_CFG_SDS_MODE_MASK, RTL8367D_PORT_SDS_MODE_1000X_100FX)) != RT_ERR_OK)
                        return retVal;
                    break;
                case MODE_EXT_1000X:
                    if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_DATA, 0x1140)) != RT_ERR_OK)
                        return retVal;

                    if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_ADR, 0x0040)) != RT_ERR_OK)
                        return retVal;

                    if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_CMD, 0x00CD)) != RT_ERR_OK)
                        return retVal;

                    if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS_MISC, RTL8367D_CFG_SDS_MODE_MASK, RTL8367D_PORT_SDS_MODE_1000X)) != RT_ERR_OK)
                        return retVal;
                    break;
                case MODE_EXT_100FX:
                    if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_DATA, 0x2100)) != RT_ERR_OK)
                        return retVal;

                    if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_ADR, 0x0040)) != RT_ERR_OK)
                        return retVal;

                    if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_CMD, 0x00CD)) != RT_ERR_OK)
                        return retVal;

                    if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS_MISC, RTL8367D_CFG_SDS_MODE_MASK, RTL8367D_PORT_SDS_MODE_100FX)) != RT_ERR_OK)
                        return retVal;
                    break;
                default:
                    return RT_ERR_INPUT;
            }
        }
        else
            return RT_ERR_INPUT;
    }

    if (id == 2)
    {
        if (mode == MODE_EXT_DISABLE)
        {
            if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_TOP_CON0, RTL8367D_MAC7_SEL_EXT1_OFFSET, 0)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_DIGITAL_INTERFACE_SELECT, RTL8367D_SELECT_GMII_1_MASK, 0x0)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS1_MISC0, RTL8367D_SDS1_MODE_MASK, RTL8367D_PORT_SDS_MODE_DISABLE)) != RT_ERR_OK)
                return retVal;
        }
        else if ((mode == MODE_EXT_RGMII) || (mode == MODE_EXT_MII_MAC) || (mode == MODE_EXT_MII_PHY) || (mode == MODE_EXT_TMII_MAC) ||
                 (mode == MODE_EXT_TMII_PHY) || (mode == MODE_EXT_RMII_MAC) || (mode == MODE_EXT_RMII_PHY))
        {
            /* Make sure RGMII/MII mux is not at port 4 */
            if ((retVal = rtl8367d_getAsicRegBit(RTL8367D_REG_TOP_CON0, RTL8367D_MAC4_SEL_EXT1_OFFSET, &mux)) != RT_ERR_OK)
                return retVal;

            if (mux == 1)
                return RT_ERR_INPUT;

            /* Configure RGMII DP, DN, E2, MODE */
            if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_CHIP_DEBUG0, RTL8367D_SEL33_EXT1_OFFSET, 1)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_CHIP_DEBUG0, RTL8367D_DRI_EXT1_RG_OFFSET, 1)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_CHIP_DEBUG0, RTL8367D_DRI_EXT1_OFFSET, 1)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_CHIP_DEBUG0, RTL8367D_SLR_EXT1_OFFSET, 1)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_CHIP_DEBUG1, RTL8367D_RG1_DN_MASK, 7)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_CHIP_DEBUG1, RTL8367D_RG1_DP_MASK, 5)) != RT_ERR_OK)
                return retVal;

            /* Configure RGMII/MII mux to port 7 */
            if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_TOP_CON0, RTL8367D_MAC7_SEL_EXT1_OFFSET, 1)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_DIGITAL_INTERFACE_SELECT, RTL8367D_SELECT_GMII_1_MASK, mode)) != RT_ERR_OK)
                return retVal;

            if((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS1_MISC0, RTL8367D_SDS1_MODE_MASK, RTL8367D_PORT_SDS_MODE_DISABLE)) != RT_ERR_OK)
                return retVal;
        }
        else if ((mode == MODE_EXT_SGMII) || (mode == MODE_EXT_HSGMII) || (mode == MODE_EXT_1000X_100FX) || (mode == MODE_EXT_1000X) || (mode == MODE_EXT_100FX))
        {
            if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_DIGITAL_INTERFACE_SELECT, RTL8367D_SELECT_GMII_1_MASK, 0x0)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_TOP_CON0, RTL8367D_MAC7_SEL_EXT1_OFFSET, 0)) != RT_ERR_OK)
                return retVal;

            if((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_SDS1_MISC0, RTL8367D_PA12PC_EN_S1_OFFSET, 1)) != RT_ERR_OK)
                return retVal;

            if((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_SDS1_MISC0, RTL8367D_PA33PC_EN_S1_OFFSET, 1)) != RT_ERR_OK)
                return retVal;

            if((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_FIBER_CFG_2, RTL8367D_SDS_RX_DISABLE_MASK, 0x1)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_UTP_FIB_DET, RTL8367D_UTP_LINK_TIMER_MASK, 0x3)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_UTP_FIB_DET, RTL8367D_UTP_SDET_TIMER_MASK, 0x2)) != RT_ERR_OK)
                return retVal;

            switch (mode)
            {
                case MODE_EXT_SGMII:
                    for(i = 0; i < sizeof(SGCfg) / (sizeof(rtk_uint32) * 2); i++)
                    {
                        if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_DATA, SGCfg[i][1])) != RT_ERR_OK)
                            return retVal;

                        if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_ADR, SGCfg[i][0])) != RT_ERR_OK)
                            return retVal;

                        if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_CMD, 0x00CF)) != RT_ERR_OK)
                            return retVal;
                    }

                    if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS1_MISC0, RTL8367D_SDS1_MODE_MASK, RTL8367D_PORT_SDS_MODE_SGMII)) != RT_ERR_OK)
                        return retVal;
                    break;
                case MODE_EXT_HSGMII:
                    for(i = 0; i < sizeof(HSGCfg) / (sizeof(rtk_uint32) * 2); i++)
                    {
                        if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_DATA, HSGCfg[i][1])) != RT_ERR_OK)
                            return retVal;

                        if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_ADR, HSGCfg[i][0])) != RT_ERR_OK)
                            return retVal;

                        if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_CMD, 0x00CF)) != RT_ERR_OK)
                            return retVal;
                    }

                    if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS1_MISC0, RTL8367D_SDS1_MODE_MASK, RTL8367D_PORT_SDS_MODE_HSGMII)) != RT_ERR_OK)
                        return retVal;
                    break;
                case MODE_EXT_1000X_100FX:
                    if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_DATA, 0x1140)) != RT_ERR_OK)
                        return retVal;

                    if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_ADR, 0x0040)) != RT_ERR_OK)
                        return retVal;

                    if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_CMD, 0x00CF)) != RT_ERR_OK)
                        return retVal;

                    if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS1_MISC0, RTL8367D_SDS1_MODE_MASK, RTL8367D_PORT_SDS_MODE_1000X_100FX)) != RT_ERR_OK)
                        return retVal;
                    break;
                case MODE_EXT_1000X:
                    if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_DATA, 0x1140)) != RT_ERR_OK)
                        return retVal;

                    if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_ADR, 0x0040)) != RT_ERR_OK)
                        return retVal;

                    if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_CMD, 0x00CF)) != RT_ERR_OK)
                        return retVal;

                    if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS1_MISC0, RTL8367D_SDS1_MODE_MASK, RTL8367D_PORT_SDS_MODE_1000X)) != RT_ERR_OK)
                        return retVal;
                    break;
                case MODE_EXT_100FX:
                    if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_DATA, 0x2100)) != RT_ERR_OK)
                        return retVal;

                    if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_ADR, 0x0040)) != RT_ERR_OK)
                        return retVal;

                    if( (retVal = rtl8367d_setAsicReg(RTL8367D_REG_SDS_INDACS_CMD, 0x00CF)) != RT_ERR_OK)
                        return retVal;

                    if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_SDS1_MISC0, RTL8367D_SDS1_MODE_MASK, RTL8367D_PORT_SDS_MODE_100FX)) != RT_ERR_OK)
                        return retVal;
                    break;
                default:
                    return RT_ERR_INPUT;
            }
        }
        else
            return RT_ERR_INPUT;
    }

    return RT_ERR_OK;
}

static rtk_api_ret_t _dal_rtl8367d_getAsicPortExtMode(rtk_uint32 id, rtk_uint32 *pMode)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    switch (id)
    {
        case 1:
            if( (retVal = rtl8367d_getAsicRegBits(RTL8367D_REG_SDS_MISC, RTL8367D_CFG_SDS_MODE_MASK, &regData)) != RT_ERR_OK)
                return retVal;
            break;
        case 2:
            if( (retVal = rtl8367d_getAsicRegBits(RTL8367D_REG_SDS1_MISC0, RTL8367D_SDS1_MODE_MASK, &regData)) != RT_ERR_OK)
                return retVal;
            break;
        default:
            return RT_ERR_OUT_OF_RANGE;
    }

    switch (regData)
    {
        case RTL8367D_PORT_SDS_MODE_SGMII:
            *pMode = MODE_EXT_SGMII;
            break;
        case RTL8367D_PORT_SDS_MODE_HSGMII:
            *pMode = MODE_EXT_HSGMII;
            break;
        case RTL8367D_PORT_SDS_MODE_1000X:
            *pMode = MODE_EXT_1000X;
            break;
        case RTL8367D_PORT_SDS_MODE_100FX:
            *pMode = MODE_EXT_100FX;
            break;
        case RTL8367D_PORT_SDS_MODE_1000X_100FX  :
            *pMode = MODE_EXT_1000X_100FX;
            break;
        case RTL8367D_PORT_SDS_MODE_DISABLE:
            if(id == 2)
            {
                if ((retVal = rtl8367d_getAsicRegBits(RTL8367D_REG_DIGITAL_INTERFACE_SELECT, RTL8367D_SELECT_GMII_1_MASK, pMode)) != RT_ERR_OK)
                    return retVal;
            }
            else
                *pMode = MODE_EXT_DISABLE;
            break;
        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_phyAutoNegoAbility_set
 * Description:
 *      Set ethernet PHY auto-negotiation desired ability.
 * Input:
 *      port        - port id.
 *      pAbility    - Ability structure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      If Full_1000 bit is set to 1, the AutoNegotiation will be automatic set to 1. While both AutoNegotiation and Full_1000 are set to 0, the PHY speed and duplex selection will
 *      be set as following 100F > 100H > 10F > 10H priority sequence.
 */
rtk_api_ret_t dal_rtl8367d_port_phyAutoNegoAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtk_api_ret_t       retVal;
    rtk_uint32          phyData;
    rtk_uint32          phyEnMsk0;
    rtk_uint32          phyEnMsk4;
    rtk_uint32          phyEnMsk9;
    rtk_port_media_t    media_type;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if(NULL == pAbility)
        return RT_ERR_NULL_POINTER;

    if (pAbility->Half_10 >= RTK_ENABLE_END || pAbility->Full_10 >= RTK_ENABLE_END ||
       pAbility->Half_100 >= RTK_ENABLE_END || pAbility->Full_100 >= RTK_ENABLE_END ||
       pAbility->Full_1000 >= RTK_ENABLE_END || pAbility->AutoNegotiation >= RTK_ENABLE_END ||
       pAbility->AsyFC >= RTK_ENABLE_END || pAbility->FC >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if (rtk_switch_isComboPort(port) == RT_ERR_OK)
    {
        if ((retVal = dal_rtl8367d_port_phyComboPortMedia_get(port, &media_type)) != RT_ERR_OK)
            return retVal;

        if(media_type == PORT_MEDIA_FIBER)
        {
            return _dal_rtl8367d_port_FiberModeAbility_set(port, pAbility);
        }
    }

    /*for PHY auto mode setup*/
    pAbility->AutoNegotiation = 1;

    phyEnMsk0 = 0;
    phyEnMsk4 = 0;
    phyEnMsk9 = 0;

    if (1 == pAbility->Half_10)
    {
        /*10BASE-TX half duplex capable in reg 4.5*/
        phyEnMsk4 = phyEnMsk4 | (1 << 5);

        /*Speed selection [1:0] */
        /* 11=Reserved*/
        /* 10= 1000Mpbs*/
        /* 01= 100Mpbs*/
        /* 00= 10Mpbs*/
        phyEnMsk0 = phyEnMsk0 & (~(1 << 6));
        phyEnMsk0 = phyEnMsk0 & (~(1 << 13));
    }

    if (1 == pAbility->Full_10)
    {
        /*10BASE-TX full duplex capable in reg 4.6*/
        phyEnMsk4 = phyEnMsk4 | (1 << 6);
        /*Speed selection [1:0] */
        /* 11=Reserved*/
        /* 10= 1000Mpbs*/
        /* 01= 100Mpbs*/
        /* 00= 10Mpbs*/
        phyEnMsk0 = phyEnMsk0 & (~(1 << 6));
        phyEnMsk0 = phyEnMsk0 & (~(1 << 13));

        /*Full duplex mode in reg 0.8*/
        phyEnMsk0 = phyEnMsk0 | (1 << 8);

    }

    if (1 == pAbility->Half_100)
    {
        /*100BASE-TX half duplex capable in reg 4.7*/
        phyEnMsk4 = phyEnMsk4 | (1 << 7);
        /*Speed selection [1:0] */
        /* 11=Reserved*/
        /* 10= 1000Mpbs*/
        /* 01= 100Mpbs*/
        /* 00= 10Mpbs*/
        phyEnMsk0 = phyEnMsk0 & (~(1 << 6));
        phyEnMsk0 = phyEnMsk0 | (1 << 13);
    }


    if (1 == pAbility->Full_100)
    {
        /*100BASE-TX full duplex capable in reg 4.8*/
        phyEnMsk4 = phyEnMsk4 | (1 << 8);
        /*Speed selection [1:0] */
        /* 11=Reserved*/
        /* 10= 1000Mpbs*/
        /* 01= 100Mpbs*/
        /* 00= 10Mpbs*/
        phyEnMsk0 = phyEnMsk0 & (~(1 << 6));
        phyEnMsk0 = phyEnMsk0 | (1 << 13);
        /*Full duplex mode in reg 0.8*/
        phyEnMsk0 = phyEnMsk0 | (1 << 8);
    }


    if (1 == pAbility->Full_1000)
    {
        /*1000 BASE-T FULL duplex capable setting in reg 9.9*/
        phyEnMsk9 = phyEnMsk9 | (1 << 9);

        /*Speed selection [1:0] */
        /* 11=Reserved*/
        /* 10= 1000Mpbs*/
        /* 01= 100Mpbs*/
        /* 00= 10Mpbs*/
        phyEnMsk0 = phyEnMsk0 | (1 << 6);
        phyEnMsk0 = phyEnMsk0 & (~(1 << 13));


        /*Auto-Negotiation setting in reg 0.12*/
        phyEnMsk0 = phyEnMsk0 | (1 << 12);

     }

    if (1 == pAbility->AutoNegotiation)
    {
        /*Auto-Negotiation setting in reg 0.12*/
        phyEnMsk0 = phyEnMsk0 | (1 << 12);
    }

    if (1 == pAbility->AsyFC)
    {
        /*Asymetric flow control in reg 4.11*/
        phyEnMsk4 = phyEnMsk4 | (1 << 11);
    }
    if (1 == pAbility->FC)
    {
        /*Flow control in reg 4.10*/
        phyEnMsk4 = phyEnMsk4 | (1 << 10);
    }

    /*1000 BASE-T control register setting*/
    if ((retVal = _dal_rtl8367d_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_1000_BASET_CONTROL_REG, &phyData)) != RT_ERR_OK)
        return retVal;

    phyData = (phyData & (~0x0200)) | phyEnMsk9 ;

    if ((retVal = _dal_rtl8367d_setAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_1000_BASET_CONTROL_REG, phyData)) != RT_ERR_OK)
        return retVal;

    /*Auto-Negotiation control register setting*/
    if ((retVal = _dal_rtl8367d_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_AN_ADVERTISEMENT_REG, &phyData)) != RT_ERR_OK)
        return retVal;

    phyData = (phyData & (~0x0DE0)) | phyEnMsk4;
    if ((retVal = _dal_rtl8367d_setAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_AN_ADVERTISEMENT_REG, phyData)) != RT_ERR_OK)
        return retVal;

    /*Control register setting and restart auto*/
    if ((retVal = _dal_rtl8367d_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
        return retVal;

    phyData = (phyData & (~0x3140)) | phyEnMsk0;
    /*If have auto-negotiation capable, then restart auto negotiation*/
    if (1 == pAbility->AutoNegotiation)
    {
        phyData = phyData | (1 << 9);
    }

    if ((retVal = _dal_rtl8367d_setAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_phyAutoNegoAbility_get
 * Description:
 *      Get PHY ability through PHY registers.
 * Input:
 *      port - Port id.
 * Output:
 *      pAbility - Ability structure
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      Get the capablity of specified PHY.
 */
rtk_api_ret_t dal_rtl8367d_port_phyAutoNegoAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtk_api_ret_t       retVal;
    rtk_uint32          phyData0;
    rtk_uint32          phyData4;
    rtk_uint32          phyData9;
    rtk_port_media_t    media_type;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if(NULL == pAbility)
        return RT_ERR_NULL_POINTER;

    if (rtk_switch_isComboPort(port) == RT_ERR_OK)
    {
        if ((retVal = dal_rtl8367d_port_phyComboPortMedia_get(port, &media_type)) != RT_ERR_OK)
            return retVal;

        if(media_type == PORT_MEDIA_FIBER)
        {
            return _dal_rtl8367d_port_FiberModeAbility_get(port, pAbility);
        }
    }

    /*Control register setting and restart auto*/
    if ((retVal = _dal_rtl8367d_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return retVal;

    /*Auto-Negotiation control register setting*/
    if ((retVal = _dal_rtl8367d_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
        return retVal;

    /*1000 BASE-T control register setting*/
    if ((retVal = _dal_rtl8367d_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_1000_BASET_CONTROL_REG, &phyData9)) != RT_ERR_OK)
        return retVal;

    if (phyData9 & (1 << 9))
        pAbility->Full_1000 = 1;
    else
        pAbility->Full_1000 = 0;

    if (phyData4 & (1 << 11))
        pAbility->AsyFC = 1;
    else
        pAbility->AsyFC = 0;

    if (phyData4 & (1 << 10))
        pAbility->FC = 1;
    else
        pAbility->FC = 0;


    if (phyData4 & (1 << 8))
        pAbility->Full_100 = 1;
    else
        pAbility->Full_100 = 0;

    if (phyData4 & (1 << 7))
        pAbility->Half_100 = 1;
    else
        pAbility->Half_100 = 0;

    if (phyData4 & (1 << 6))
        pAbility->Full_10 = 1;
    else
        pAbility->Full_10 = 0;

    if (phyData4 & (1 << 5))
        pAbility->Half_10 = 1;
    else
        pAbility->Half_10 = 0;


    if (phyData0 & (1 << 12))
        pAbility->AutoNegotiation = 1;
    else
        pAbility->AutoNegotiation = 0;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_phyForceModeAbility_set
 * Description:
 *      Set the port speed/duplex mode/pause/asy_pause in the PHY force mode.
 * Input:
 *      port        - port id.
 *      pAbility    - Ability structure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      While both AutoNegotiation and Full_1000 are set to 0, the PHY speed and duplex selection will
 *      be set as following 100F > 100H > 10F > 10H priority sequence.
 *      This API can be used to configure combo port in fiber mode.
 *      The possible parameters in fiber mode are Full_1000 and Full 100.
 *      All the other fields in rtk_port_phy_ability_t will be ignored in fiber port.
 */
rtk_api_ret_t dal_rtl8367d_port_phyForceModeAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
     rtk_api_ret_t      retVal;
     rtk_uint32         phyData;
     rtk_uint32         phyEnMsk0;
     rtk_uint32         phyEnMsk4;
     rtk_uint32         phyEnMsk9;
     rtk_port_media_t   media_type;

     /* Check initialization state */
     RTK_CHK_INIT_STATE();

     /* Check Port Valid */
     RTK_CHK_PORT_IS_UTP(port);

     if(NULL == pAbility)
        return RT_ERR_NULL_POINTER;

     if (pAbility->Half_10 >= RTK_ENABLE_END || pAbility->Full_10 >= RTK_ENABLE_END ||
        pAbility->Half_100 >= RTK_ENABLE_END || pAbility->Full_100 >= RTK_ENABLE_END ||
        pAbility->Full_1000 >= RTK_ENABLE_END || pAbility->AutoNegotiation >= RTK_ENABLE_END ||
        pAbility->AsyFC >= RTK_ENABLE_END || pAbility->FC >= RTK_ENABLE_END)
         return RT_ERR_INPUT;

     if (rtk_switch_isComboPort(port) == RT_ERR_OK)
     {
         if ((retVal = dal_rtl8367d_port_phyComboPortMedia_get(port, &media_type)) != RT_ERR_OK)
             return retVal;

         if(media_type == PORT_MEDIA_FIBER)
         {
             return _dal_rtl8367d_port_FiberModeAbility_set(port, pAbility);
         }
     }

     if (1 == pAbility->Full_1000)
         return RT_ERR_INPUT;

     /*for PHY force mode setup*/
     pAbility->AutoNegotiation = 0;

     phyEnMsk0 = 0;
     phyEnMsk4 = 0;
     phyEnMsk9 = 0;

     if (1 == pAbility->Half_10)
     {
         /*10BASE-TX half duplex capable in reg 4.5*/
         phyEnMsk4 = phyEnMsk4 | (1 << 5);

         /*Speed selection [1:0] */
         /* 11=Reserved*/
         /* 10= 1000Mpbs*/
         /* 01= 100Mpbs*/
         /* 00= 10Mpbs*/
         phyEnMsk0 = phyEnMsk0 & (~(1 << 6));
         phyEnMsk0 = phyEnMsk0 & (~(1 << 13));
     }

     if (1 == pAbility->Full_10)
     {
         /*10BASE-TX full duplex capable in reg 4.6*/
         phyEnMsk4 = phyEnMsk4 | (1 << 6);
         /*Speed selection [1:0] */
         /* 11=Reserved*/
         /* 10= 1000Mpbs*/
         /* 01= 100Mpbs*/
         /* 00= 10Mpbs*/
         phyEnMsk0 = phyEnMsk0 & (~(1 << 6));
         phyEnMsk0 = phyEnMsk0 & (~(1 << 13));

         /*Full duplex mode in reg 0.8*/
         phyEnMsk0 = phyEnMsk0 | (1 << 8);

     }

     if (1 == pAbility->Half_100)
     {
         /*100BASE-TX half duplex capable in reg 4.7*/
         phyEnMsk4 = phyEnMsk4 | (1 << 7);
         /*Speed selection [1:0] */
         /* 11=Reserved*/
         /* 10= 1000Mpbs*/
         /* 01= 100Mpbs*/
         /* 00= 10Mpbs*/
         phyEnMsk0 = phyEnMsk0 & (~(1 << 6));
         phyEnMsk0 = phyEnMsk0 | (1 << 13);
     }


     if (1 == pAbility->Full_100)
     {
         /*100BASE-TX full duplex capable in reg 4.8*/
         phyEnMsk4 = phyEnMsk4 | (1 << 8);
         /*Speed selection [1:0] */
         /* 11=Reserved*/
         /* 10= 1000Mpbs*/
         /* 01= 100Mpbs*/
         /* 00= 10Mpbs*/
         phyEnMsk0 = phyEnMsk0 & (~(1 << 6));
         phyEnMsk0 = phyEnMsk0 | (1 << 13);
         /*Full duplex mode in reg 0.8*/
         phyEnMsk0 = phyEnMsk0 | (1 << 8);
     }

     if (1 == pAbility->AsyFC)
     {
         /*Asymetric flow control in reg 4.11*/
         phyEnMsk4 = phyEnMsk4 | (1 << 11);
     }
     if (1 == pAbility->FC)
     {
         /*Flow control in reg 4.10*/
         phyEnMsk4 = phyEnMsk4 | ((1 << 10));
     }

     /*1000 BASE-T control register setting*/
     if ((retVal = _dal_rtl8367d_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_1000_BASET_CONTROL_REG, &phyData)) != RT_ERR_OK)
         return retVal;

     phyData = (phyData & (~0x0200)) | phyEnMsk9 ;

     if ((retVal = _dal_rtl8367d_setAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_1000_BASET_CONTROL_REG, phyData)) != RT_ERR_OK)
         return retVal;

     /*Auto-Negotiation control register setting*/
     if ((retVal = _dal_rtl8367d_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_AN_ADVERTISEMENT_REG, &phyData)) != RT_ERR_OK)
         return retVal;

     phyData = (phyData & (~0x0DE0)) | phyEnMsk4;
     if ((retVal = _dal_rtl8367d_setAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_AN_ADVERTISEMENT_REG, phyData)) != RT_ERR_OK)
         return retVal;

     /*Control register setting and power off/on*/
     phyData = phyEnMsk0 & (~(1 << 12));
     phyData |= (1 << 11);   /* power down PHY, bit 11 should be set to 1 */
     if ((retVal = _dal_rtl8367d_setAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
         return retVal;

     phyData = phyData & (~(1 << 11));   /* power on PHY, bit 11 should be set to 0*/
     if ((retVal = _dal_rtl8367d_setAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
         return retVal;

     return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_phyForceModeAbility_get
 * Description:
 *      Get PHY ability through PHY registers.
 * Input:
 *      port - Port id.
 * Output:
 *      pAbility - Ability structure
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      Get the capablity of specified PHY.
 */
rtk_api_ret_t dal_rtl8367d_port_phyForceModeAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtk_api_ret_t       retVal;
    rtk_uint32          phyData0;
    rtk_uint32          phyData4;
    rtk_uint32          phyData9;
    rtk_port_media_t    media_type;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
     RTK_CHK_PORT_IS_UTP(port);

     if(NULL == pAbility)
        return RT_ERR_NULL_POINTER;

     if (rtk_switch_isComboPort(port) == RT_ERR_OK)
     {
         if ((retVal = dal_rtl8367d_port_phyComboPortMedia_get(port, &media_type)) != RT_ERR_OK)
             return retVal;

         if(media_type == PORT_MEDIA_FIBER)
         {
             return _dal_rtl8367d_port_FiberModeAbility_get(port, pAbility);
         }
     }

    /*Control register setting and restart auto*/
    if ((retVal = _dal_rtl8367d_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return retVal;

    /*Auto-Negotiation control register setting*/
    if ((retVal = _dal_rtl8367d_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
        return retVal;

    /*1000 BASE-T control register setting*/
    if ((retVal = _dal_rtl8367d_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_1000_BASET_CONTROL_REG, &phyData9)) != RT_ERR_OK)
        return retVal;

    if (phyData9 & (1 << 9))
        pAbility->Full_1000 = 1;
    else
        pAbility->Full_1000 = 0;

    if (phyData4 & (1 << 11))
        pAbility->AsyFC = 1;
    else
        pAbility->AsyFC = 0;

    if (phyData4 & ((1 << 10)))
        pAbility->FC = 1;
    else
        pAbility->FC = 0;


    if (phyData4 & (1 << 8))
        pAbility->Full_100 = 1;
    else
        pAbility->Full_100 = 0;

    if (phyData4 & (1 << 7))
        pAbility->Half_100 = 1;
    else
        pAbility->Half_100 = 0;

    if (phyData4 & (1 << 6))
        pAbility->Full_10 = 1;
    else
        pAbility->Full_10 = 0;

    if (phyData4 & (1 << 5))
        pAbility->Half_10 = 1;
    else
        pAbility->Half_10 = 0;


    if (phyData0 & (1 << 12))
        pAbility->AutoNegotiation = 1;
    else
        pAbility->AutoNegotiation = 0;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_phyStatus_get
 * Description:
 *      Get ethernet PHY linking status
 * Input:
 *      port - Port id.
 * Output:
 *      linkStatus  - PHY link status
 *      speed       - PHY link speed
 *      duplex      - PHY duplex mode
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      API will return auto negotiation status of phy.
 */
rtk_api_ret_t dal_rtl8367d_port_phyStatus_get(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus, rtk_port_speed_t *pSpeed, rtk_port_duplex_t *pDuplex)
{
    rtk_api_ret_t retVal;
    rtk_uint32 phyData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if( (NULL == pLinkStatus) || (NULL == pSpeed) || (NULL == pDuplex) )
        return RT_ERR_NULL_POINTER;

    /*Get PHY resolved register*/
    if ((retVal = _dal_rtl8367d_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_RESOLVED_REG, &phyData)) != RT_ERR_OK)
        return retVal;

    /*check link status*/
    if (phyData & (1<<2))
    {
        *pLinkStatus = 1;

        /*check link speed*/
        *pSpeed = (phyData&0x0030) >> 4;

        /*check link duplex*/
        *pDuplex = (phyData&0x0008) >> 3;
    }
    else
    {
        *pLinkStatus = 0;
        *pSpeed = 0;
        *pDuplex = 0;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_macForceLink_set
 * Description:
 *      Set port force linking configuration.
 * Input:
 *      port            - port id.
 *      pPortability    - port ability configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can set Port/MAC force mode properties.
 */
rtk_api_ret_t dal_rtl8367d_port_macForceLink_set(rtk_port_t port, rtk_port_mac_ability_t *pPortability)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if(NULL == pPortability)
        return RT_ERR_NULL_POINTER;

    if (pPortability->forcemode >1|| pPortability->speed > 2 || pPortability->duplex > 1 ||
       pPortability->link > 1 || pPortability->nway > 1 || pPortability->txpause > 1 || pPortability->rxpause > 1)
        return RT_ERR_INPUT;

    if((retVal = rtl8367d_getAsicReg(RTL8367D_REG_MAC0_FORCE_SELECT + rtk_switch_port_L2P_get(port), &regData)) != RT_ERR_OK)
        return retVal;

    regData &= ~(0x0001 << 12);
    regData |= pPortability->forcemode << 12;

    regData &= ~(0x0001 << 7);
    regData |= pPortability->nway << 7;

    regData &= ~(0x0001 << 6);
    regData |= pPortability->txpause << 6;

    regData &= ~(0x0001 << 5);
    regData |= pPortability->rxpause << 5;

    regData &= ~(0x0001 << 4);
    regData |= pPortability->link << 4;

    regData &= ~(0x0001 << 2);
    regData |= pPortability->duplex << 2;

    regData &= ~(0x0003);
    regData |= pPortability->speed;

    if((retVal = rtl8367d_setAsicReg(RTL8367D_REG_MAC0_FORCE_SELECT + rtk_switch_port_L2P_get(port), regData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_macForceLink_get
 * Description:
 *      Get port force linking configuration.
 * Input:
 *      port - Port id.
 * Output:
 *      pPortability - port ability configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can get Port/MAC force mode properties.
 */
rtk_api_ret_t dal_rtl8367d_port_macForceLink_get(rtk_port_t port, rtk_port_mac_ability_t *pPortability)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if(NULL == pPortability)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8367d_getAsicReg(RTL8367D_REG_MAC0_FORCE_SELECT + rtk_switch_port_L2P_get(port), &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pPortability->forcemode = (regData >> 12) & 0x0001;
    pPortability->nway      = (regData >> 7) & 0x0001;
    pPortability->txpause   = (regData >> 6) & 0x0001;
    pPortability->rxpause   = (regData >> 5) & 0x0001;
    pPortability->link      = (regData >> 4) & 0x0001;
    pPortability->duplex    = (regData >> 2) & 0x0001;
    pPortability->speed     = regData & 0x0003;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_macForceLinkExt_set
 * Description:
 *      Set external interface force linking configuration.
 * Input:
 *      port            - external port ID
 *      mode            - external interface mode
 *      pPortability    - port ability configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can set external interface force mode properties.
 *      The external interface can be set to:
 *      - MODE_EXT_DISABLE,
 *      - MODE_EXT_RGMII,
 *      - MODE_EXT_MII_MAC,
 *      - MODE_EXT_MII_PHY,
 *      - MODE_EXT_TMII_MAC,
 *      - MODE_EXT_TMII_PHY,
 *      - MODE_EXT_GMII,
 *      - MODE_EXT_RMII_MAC,
 *      - MODE_EXT_RMII_PHY,
 *      - MODE_EXT_SGMII,
 *      - MODE_EXT_HSGMII,
 */
rtk_api_ret_t dal_rtl8367d_port_macForceLinkExt_set(rtk_port_t port, rtk_mode_ext_t mode, rtk_port_mac_ability_t *pPortability)
{
    rtk_api_ret_t retVal;
    rtl8367d_port_ability_t ability;
    rtk_uint32 ext_id;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_EXT(port);

    if(NULL == pPortability)
        return RT_ERR_NULL_POINTER;

    if (mode > MODE_EXT_100FX)
        return RT_ERR_INPUT;

    if (mode == MODE_EXT_GMII)
        return RT_ERR_INPUT;

    if(mode == MODE_EXT_HSGMII)
    {
        if (pPortability->forcemode > 1 || pPortability->speed != PORT_SPEED_2500M || pPortability->duplex != PORT_FULL_DUPLEX ||
           pPortability->link >= PORT_LINKSTATUS_END || pPortability->nway > 1 || pPortability->txpause > 1 || pPortability->rxpause > 1)
            return RT_ERR_INPUT;

        if(rtk_switch_isHsgPort(port) != RT_ERR_OK)
            return RT_ERR_PORT_ID;
    }
    else if (mode == MODE_EXT_RGMII)
    {
        if (pPortability->forcemode > 1 || pPortability->speed > PORT_SPEED_1000M || pPortability->duplex >= PORT_DUPLEX_END ||
           pPortability->link >= PORT_LINKSTATUS_END || pPortability->nway > 1 || pPortability->txpause > 1 || pPortability->rxpause > 1)
            return RT_ERR_INPUT;

        if (port != EXT_PORT1)
            return RT_ERR_PORT_ID;
    }
    else if (mode != MODE_EXT_DISABLE)
    {
        if (pPortability->forcemode > 1 || pPortability->speed > PORT_SPEED_1000M || pPortability->duplex >= PORT_DUPLEX_END ||
           pPortability->link >= PORT_LINKSTATUS_END || pPortability->nway > 1 || pPortability->txpause > 1 || pPortability->rxpause > 1)
            return RT_ERR_INPUT;
    }

    ext_id = port - 15;

    if(mode == MODE_EXT_DISABLE)
    {
        /* Clear Ability */
        memset(&ability, 0x00, sizeof(rtl8367d_port_ability_t));
        if((retVal = _dal_rtl8367d_setAsicPortForceLinkExt(ext_id, &ability)) != RT_ERR_OK)
            return retVal;

        /* Set mode to Disable */
        if ((retVal = _dal_rtl8367d_setAsicPortExtMode(ext_id, mode)) != RT_ERR_OK)
            return retVal;
    }
    else
    {
        /* Configure EXT port mode */
        if ((retVal = _dal_rtl8367d_setAsicPortExtMode(ext_id, mode)) != RT_ERR_OK)
            return retVal;

        /* Configure Ability */
        if ((retVal = _dal_rtl8367d_getAsicPortForceLinkExt(ext_id, &ability)) != RT_ERR_OK)
            return retVal;

        ability.forcemode = pPortability->forcemode;
        ability.duplex    = pPortability->duplex;
        ability.link      = pPortability->link;
        ability.nway      = pPortability->nway;
        ability.txpause   = pPortability->txpause;
        ability.rxpause   = pPortability->rxpause;

        switch (pPortability->speed)
        {
            case PORT_SPEED_10M:
                ability.speed = RTL8367D_EXT_PORT_SPEED_10M;
                break;
            case PORT_SPEED_100M:
                ability.speed = RTL8367D_EXT_PORT_SPEED_100M;
                break;
            case PORT_SPEED_1000M:
                ability.speed = RTL8367D_EXT_PORT_SPEED_1000M;
                break;
            case PORT_SPEED_500M:
                ability.speed = RTL8367D_EXT_PORT_SPEED_500M;
                break;
            case PORT_SPEED_2500M:
                ability.speed = RTL8367D_EXT_PORT_SPEED_2500M;
                break;
            default:
                return RT_ERR_INPUT;
        }

        if ((retVal = _dal_rtl8367d_setAsicPortForceLinkExt(ext_id, &ability)) != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_macForceLinkExt_get
 * Description:
 *      Set external interface force linking configuration.
 * Input:
 *      port            - external port ID
 * Output:
 *      pMode           - external interface mode
 *      pPortability    - port ability configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can get external interface force mode properties.
 */
rtk_api_ret_t dal_rtl8367d_port_macForceLinkExt_get(rtk_port_t port, rtk_mode_ext_t *pMode, rtk_port_mac_ability_t *pPortability)
{
    rtk_api_ret_t retVal;
    rtl8367d_port_ability_t ability;
    rtk_uint32 ext_id;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_EXT(port);

    if(NULL == pMode)
        return RT_ERR_NULL_POINTER;

    if(NULL == pPortability)
        return RT_ERR_NULL_POINTER;

    ext_id = port - 15;

    if ((retVal = _dal_rtl8367d_getAsicPortExtMode(ext_id, (rtk_uint32 *)pMode)) != RT_ERR_OK)
        return retVal;

    if ((retVal = _dal_rtl8367d_getAsicPortForceLinkExt(ext_id, &ability)) != RT_ERR_OK)
        return retVal;

    pPortability->forcemode = ability.forcemode;
    pPortability->duplex    = ability.duplex;
    pPortability->link      = ability.link;
    pPortability->nway      = ability.nway;
    pPortability->txpause   = ability.txpause;
    pPortability->rxpause   = ability.rxpause;

    switch (ability.speed)
    {
        case RTL8367D_EXT_PORT_SPEED_10M:
            pPortability->speed = PORT_SPEED_10M;
            break;
        case RTL8367D_EXT_PORT_SPEED_100M:
            pPortability->speed = PORT_SPEED_100M;
            break;
        case RTL8367D_EXT_PORT_SPEED_1000M:
            pPortability->speed = PORT_SPEED_1000M;
            break;
        case RTL8367D_EXT_PORT_SPEED_500M:
            pPortability->speed = PORT_SPEED_500M;
            break;
        case RTL8367D_EXT_PORT_SPEED_2500M:
            pPortability->speed = PORT_SPEED_2500M;
            break;
        default:
            return RT_ERR_INPUT;
    }

    return RT_ERR_OK;

}

/* Function Name:
 *      dal_rtl8367d_port_macStatus_get
 * Description:
 *      Get port link status.
 * Input:
 *      port - Port id.
 * Output:
 *      pPortstatus - port ability configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can get Port/PHY properties.
 */
rtk_api_ret_t dal_rtl8367d_port_macStatus_get(rtk_port_t port, rtk_port_mac_ability_t *pPortstatus)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;
    rtk_uint32 rtl8367d_speed;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pPortstatus)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367d_getAsicReg(RTL8367D_REG_PORT0_STATUS+rtk_switch_port_L2P_get(port), &regData)) != RT_ERR_OK)
        return retVal;

    pPortstatus->duplex    = (regData >> 2) & 0x0001;
    pPortstatus->link      = (regData >> 4) & 0x0001;
    pPortstatus->nway      = (regData >> 7) & 0x0001;
    pPortstatus->txpause   = (regData >> 6) & 0x0001;
    pPortstatus->rxpause   = (regData >> 5) & 0x0001;

    rtl8367d_speed = (regData & 0x0003) | (((regData & 0x3000) >> 12) << 2);
    switch (rtl8367d_speed)
    {
        case RTL8367D_EXT_PORT_SPEED_10M:
            pPortstatus->speed = PORT_SPEED_10M;
            break;
        case RTL8367D_EXT_PORT_SPEED_100M:
            pPortstatus->speed = PORT_SPEED_100M;
            break;
        case RTL8367D_EXT_PORT_SPEED_1000M:
            pPortstatus->speed = PORT_SPEED_1000M;
            break;
        case RTL8367D_EXT_PORT_SPEED_500M:
            pPortstatus->speed = PORT_SPEED_500M;
            break;
        case RTL8367D_EXT_PORT_SPEED_2500M:
            pPortstatus->speed = PORT_SPEED_2500M;
            break;
        default:
            return RT_ERR_INPUT;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_macLocalLoopbackEnable_set
 * Description:
 *      Set Port Local Loopback. (Redirect TX to RX.)
 * Input:
 *      port    - Port id.
 *      enable  - Loopback state, 0:disable, 1:enable
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can enable/disable Local loopback in MAC.
 *      For UTP port, This API will also enable the digital
 *      loopback bit in PHY register for sync of speed between
 *      PHY and MAC. For EXT port, users need to force the
 *      link state by themself.
 */
rtk_api_ret_t dal_rtl8367d_port_macLocalLoopbackEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      data;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_PORT0_MISC_CFG + (rtk_switch_port_L2P_get(port) << 5), RTL8367D_PORT0_MISC_CFG_MAC_LOOPBACK_OFFSET, (enable == ENABLED) ? 1 : 0)) != RT_ERR_OK)
        return retVal;

    if(rtk_switch_isUtpPort(port) == RT_ERR_OK)
    {
        if ((retVal = _dal_rtl8367d_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_CONTROL_REG, &data)) != RT_ERR_OK)
            return retVal;

        if(enable == ENABLED)
            data |= (0x0001 << 14);
        else
            data &= ~(0x0001 << 14);

        if ((retVal = _dal_rtl8367d_setAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_CONTROL_REG, data)) != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_macLocalLoopbackEnable_get
 * Description:
 *      Get Port Local Loopback. (Redirect TX to RX.)
 * Input:
 *      port    - Port id.
 * Output:
 *      pEnable  - Loopback state, 0:disable, 1:enable
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      None.
 */
rtk_api_ret_t dal_rtl8367d_port_macLocalLoopbackEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367d_getAsicRegBit(RTL8367D_REG_PORT0_MISC_CFG + (rtk_switch_port_L2P_get(port) << 5), RTL8367D_PORT0_MISC_CFG_MAC_LOOPBACK_OFFSET, &regData)) != RT_ERR_OK)
        return retVal;

    *pEnable = (regData == 1) ? ENABLED : DISABLED;
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_phyReg_set
 * Description:
 *      Set PHY register data of the specific port.
 * Input:
 *      port    - port id.
 *      reg     - Register id
 *      regData - Register data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      This API can set PHY register data of the specific port.
 */
rtk_api_ret_t dal_rtl8367d_port_phyReg_set(rtk_port_t port, rtk_port_phy_reg_t reg, rtk_port_phy_data_t regData)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if ((retVal = _dal_rtl8367d_setAsicPHYReg(rtk_switch_port_L2P_get(port), reg, regData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_phyReg_get
 * Description:
 *      Get PHY register data of the specific port.
 * Input:
 *      port    - Port id.
 *      reg     - Register id
 * Output:
 *      pData   - Register data
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      This API can get PHY register data of the specific port.
 */
rtk_api_ret_t dal_rtl8367d_port_phyReg_get(rtk_port_t port, rtk_port_phy_reg_t reg, rtk_port_phy_data_t *pData)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if ((retVal = _dal_rtl8367d_getAsicPHYReg(rtk_switch_port_L2P_get(port), reg, pData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8367d_port_phyOCPReg_set
 * Description:
 *      Set PHY OCP register
 * Input:
 *      port        - PHY ID
 *      ocpAddr     - OCP register address
 *      ocpData     - OCP Data.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK                               - OK
 *      RT_ERR_FAILED                           - Failed
 *      RT_ERR_BUSYWAIT_TIMEOUT                 - Timeout
 * Note:
 *      None.
 */
rtk_api_ret_t dal_rtl8367d_port_phyOCPReg_set(rtk_port_t port, rtk_uint32 ocpAddr, rtk_uint32 ocpData )
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if ((retVal = _dal_rtl8367d_setAsicPHYOCPReg(rtk_switch_port_L2P_get(port), ocpAddr, ocpData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_phyOCPReg_get
 * Description:
 *      Set PHY OCP register
 * Input:
 *      phyNo       - PHY ID
 *      ocpAddr     - OCP register address
 * Output:
 *      pRegData    - OCP data.
 * Return:
 *      RT_ERR_OK                               - OK
 *      RT_ERR_FAILED                           - Failed
 *      RT_ERR_NULL_POINTER                     - Null pointer
 * Note:
 *      None.
 */
rtk_api_ret_t dal_rtl8367d_port_phyOCPReg_get(rtk_port_t port, rtk_uint32 ocpAddr, rtk_uint32 *pRegData )
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if (pRegData == NULL)
        return RT_ERR_NULL_POINTER;

    if ((retVal = _dal_rtl8367d_getAsicPHYOCPReg(rtk_switch_port_L2P_get(port), ocpAddr, pRegData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8367d_port_backpressureEnable_set
 * Description:
 *      Set the half duplex backpressure enable status of the specific port.
 * Input:
 *      port    - port id.
 *      enable  - Back pressure status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can set the half duplex backpressure enable status of the specific port.
 *      The half duplex backpressure enable status of the port is as following:
 *      - DISABLE(Defer)
 *      - ENABLE (Backpressure)
 */
rtk_api_ret_t dal_rtl8367d_port_backpressureEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (port != RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_CFG_BACKPRESSURE, RTL8367D_LONGTXE_OFFSET, (enable == ENABLED) ? 0 : 1)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_backpressureEnable_get
 * Description:
 *      Get the half duplex backpressure enable status of the specific port.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Back pressure status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can get the half duplex backpressure enable status of the specific port.
 *      The half duplex backpressure enable status of the port is as following:
 *      - DISABLE(Defer)
 *      - ENABLE (Backpressure)
 */
rtk_api_ret_t dal_rtl8367d_port_backpressureEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (port != RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367d_getAsicRegBit(RTL8367D_REG_CFG_BACKPRESSURE, RTL8367D_LONGTXE_OFFSET, &regData)) != RT_ERR_OK)
        return retVal;

    *pEnable = (regData == 1) ? DISABLED : ENABLED;
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_adminEnable_set
 * Description:
 *      Set port admin configuration of the specific port.
 * Input:
 *      port    - port id.
 *      enable  - Back pressure status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can set port admin configuration of the specific port.
 *      The port admin configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t dal_rtl8367d_port_adminEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32      data;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = _dal_rtl8367d_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_CONTROL_REG, &data)) != RT_ERR_OK)
        return retVal;

    if (ENABLED == enable)
    {
        data &= 0xF7FF;
        data |= 0x0200;
    }
    else if (DISABLED == enable)
    {
        data |= 0x0800;
    }

    if ((retVal = _dal_rtl8367d_setAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_CONTROL_REG, data)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_adminEnable_get
 * Description:
 *      Get port admin configurationof the specific port.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Back pressure status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can get port admin configuration of the specific port.
 *      The port admin configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t dal_rtl8367d_port_adminEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32      data;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = _dal_rtl8367d_getAsicPHYReg(port, PHY_CONTROL_REG, &data)) != RT_ERR_OK)
        return retVal;

    if ( (data & 0x0800) == 0x0800)
    {
        *pEnable = DISABLED;
    }
    else
    {
        *pEnable = ENABLED;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_isolation_set
 * Description:
 *      Set permitted port isolation portmask
 * Input:
 *      port         - port id.
 *      pPortmask    - Permit port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_PORT_MASK    - Invalid portmask.
 * Note:
 *      This API set the port mask that a port can trasmit packet to of each port
 *      A port can only transmit packet to ports included in permitted portmask
 */
rtk_api_ret_t dal_rtl8367d_port_isolation_set(rtk_port_t port, rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    /* check port mask */
    RTK_CHK_PORTMASK_VALID(pPortmask);

    if ((retVal = rtk_switch_portmask_L2P_get(pPortmask, &pmask)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367d_setAsicReg(RTL8367D_REG_PORT_ISOLATION_PORT0_MASK + rtk_switch_port_L2P_get(port), pmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_isolation_get
 * Description:
 *      Get permitted port isolation portmask
 * Input:
 *      port - Port id.
 * Output:
 *      pPortmask - Permit port mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API get the port mask that a port can trasmit packet to of each port
 *      A port can only transmit packet to ports included in permitted portmask
 */
rtk_api_ret_t dal_rtl8367d_port_isolation_get(rtk_port_t port, rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367d_getAsicReg(RTL8367D_REG_PORT_ISOLATION_PORT0_MASK + rtk_switch_port_L2P_get(port), &pmask)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtk_switch_portmask_P2L_get(pmask, pPortmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_rgmiiDelayExt_set
 * Description:
 *      Set RGMII interface delay value for TX and RX.
 * Input:
 *      txDelay - TX delay value, 1 for delay 2ns and 0 for no-delay
 *      rxDelay - RX delay value, 0~7 for delay setup.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can set external interface 2 RGMII delay.
 *      In TX delay, there are 2 selection: no-delay and 2ns delay.
 *      In RX dekay, there are 8 steps for delay tunning. 0 for no-delay, and 7 for maximum delay.
 *      Note. This API should be called before rtk_port_macForceLinkExt_set().
 */
rtk_api_ret_t dal_rtl8367d_port_rgmiiDelayExt_set(rtk_port_t port, rtk_data_t txDelay, rtk_data_t rxDelay)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_EXT(port);

    if ((txDelay > 1) || (rxDelay > 7))
        return RT_ERR_INPUT;

    if (port != EXT_PORT1)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8367d_getAsicReg(RTL8367D_REG_EXT1_RGMXF, &regData)) != RT_ERR_OK)
        return retVal;

    regData = (regData & 0xFFF0) | ((txDelay << 3) & 0x0008) | (rxDelay & 0x0007);

    if ((retVal = rtl8367d_setAsicReg(RTL8367D_REG_EXT1_RGMXF, regData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_rgmiiDelayExt_get
 * Description:
 *      Get RGMII interface delay value for TX and RX.
 * Input:
 *      None
 * Output:
 *      pTxDelay - TX delay value
 *      pRxDelay - RX delay value
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can set external interface 2 RGMII delay.
 *      In TX delay, there are 2 selection: no-delay and 2ns delay.
 *      In RX dekay, there are 8 steps for delay tunning. 0 for n0-delay, and 7 for maximum delay.
 */
rtk_api_ret_t dal_rtl8367d_port_rgmiiDelayExt_get(rtk_port_t port, rtk_data_t *pTxDelay, rtk_data_t *pRxDelay)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_EXT(port);

    if( (NULL == pTxDelay) || (NULL == pRxDelay) )
        return RT_ERR_NULL_POINTER;

    if (port != EXT_PORT1)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8367d_getAsicReg(RTL8367D_REG_EXT1_RGMXF, &regData)) != RT_ERR_OK)
        return retVal;

    *pTxDelay = (regData & 0x0008) >> 3;
    *pRxDelay = regData & 0x0007;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_phyEnableAll_set
 * Description:
 *      Set all PHY enable status.
 * Input:
 *      enable - PHY Enable State.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can set all PHY status.
 *      The configuration of all PHY is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t dal_rtl8367d_port_phyEnableAll_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 data;
    rtk_uint32 port;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    RTK_SCAN_ALL_LOG_PORT(port)
    {
        if(rtk_switch_isUtpPort(port) == RT_ERR_OK)
        {
            if ((retVal = _dal_rtl8367d_getAsicPHYReg(port, PHY_CONTROL_REG, &data)) != RT_ERR_OK)
                return retVal;

            if (ENABLED == enable)
            {
                data &= 0xF7FF;
                data |= 0x0200;
            }
            else
            {
                data |= 0x0800;
            }

            if ((retVal = _dal_rtl8367d_setAsicPHYReg(port, PHY_CONTROL_REG, data)) != RT_ERR_OK)
                return retVal;
        }
    }

    return RT_ERR_OK;

}

/* Function Name:
 *      dal_rtl8367d_port_phyEnableAll_get
 * Description:
 *      Get all PHY enable status.
 * Input:
 *      None
 * Output:
 *      pEnable - PHY Enable State.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      This API can set all PHY status.
 *      The configuration of all PHY is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t dal_rtl8367d_port_phyEnableAll_get(rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 data;
    rtk_uint32 port;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    RTK_SCAN_ALL_LOG_PORT(port)
    {
        if(rtk_switch_isUtpPort(port) == RT_ERR_OK)
        {
            if ((retVal = _dal_rtl8367d_getAsicPHYReg(port, PHY_CONTROL_REG, &data)) != RT_ERR_OK)
                return retVal;

            if (data & 0x0800)
            {
                *pEnable = DISABLED;
                return RT_ERR_OK;
            }
        }
    }

    *pEnable = ENABLED;
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_phyComboPortMedia_set
 * Description:
 *      Set Combo port media type
 * Input:
 *      port    - Port id.
 *      media   - Media (COPPER or FIBER or AUTO)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_PORT_ID          - Invalid port ID.
 * Note:
 *      The API can Set Combo port media type.
 */
rtk_api_ret_t dal_rtl8367d_port_phyComboPortMedia_set(rtk_port_t port, rtk_port_media_t media)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    /* Check Combo Port ID */
    RTK_CHK_PORT_IS_COMBO(port);

    if (media >= PORT_MEDIA_END)
        return RT_ERR_INPUT;

    if(media == PORT_MEDIA_FIBER)
    {
        if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_UTP_FIB_DET, RTL8367D_FORCE_AUTODET_OFFSET, 1))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_UTP_FIB_DET, RTL8367D_UTP_FIB_DISAUTODET_OFFSET, 1))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_UTP_FIB_DET, RTL8367D_UTP_FIRST_OFFSET, 0))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_SDS_MISC, RTL8367D_MAC6_SEL_SDS0_OFFSET, 0))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_TOP_CON0, RTL8367D_MAC4_SEL_EXT1_OFFSET, 0))!=RT_ERR_OK)
            return retVal;
    }
    else if(media == PORT_MEDIA_AUTO)
    {
        if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_UTP_FIB_DET, RTL8367D_FORCE_AUTODET_OFFSET, 1))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_UTP_FIB_DET, RTL8367D_UTP_FIB_DISAUTODET_OFFSET, 0))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_SDS_MISC, RTL8367D_MAC6_SEL_SDS0_OFFSET, 0))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_TOP_CON0, RTL8367D_MAC4_SEL_EXT1_OFFSET, 0))!=RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_UTP_FIB_DET, RTL8367D_FORCE_AUTODET_OFFSET, 1))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_UTP_FIB_DET, RTL8367D_UTP_FIB_DISAUTODET_OFFSET, 1))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_UTP_FIB_DET, RTL8367D_UTP_FIRST_OFFSET, 1))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_SDS_MISC, RTL8367D_MAC6_SEL_SDS0_OFFSET, 1))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_TOP_CON0, RTL8367D_MAC4_SEL_EXT1_OFFSET, 0))!=RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_phyComboPortMedia_get
 * Description:
 *      Get Combo port media type
 * Input:
 *      port    - Port id.
 * Output:
 *      pMedia  - Media (COPPER or FIBER or AUTO)
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_PORT_ID          - Invalid port ID.
 * Note:
 *      The API can Set Combo port media type.
 */
rtk_api_ret_t dal_rtl8367d_port_phyComboPortMedia_get(rtk_port_t port, rtk_port_media_t *pMedia)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      data;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    /* Check Combo Port ID */
    RTK_CHK_PORT_IS_COMBO(port);

    if ((retVal = rtl8367d_getAsicRegBit(RTL8367D_REG_UTP_FIB_DET, RTL8367D_UTP_FIB_DISAUTODET_OFFSET, &data))!=RT_ERR_OK)
        return retVal;

    if(data == 0)
    {
        *pMedia = PORT_MEDIA_AUTO;
    }
    else
    {
        if ((retVal = rtl8367d_getAsicRegBit(RTL8367D_REG_UTP_FIB_DET, RTL8367D_UTP_FIRST_OFFSET, &data))!=RT_ERR_OK)
                return retVal;

        if(data == 1)
            *pMedia = PORT_MEDIA_COPPER;
        else
            *pMedia = PORT_MEDIA_FIBER;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_sgmiiLinkStatus_get
 * Description:
 *      Get SGMII status
 * Input:
 *      port        - Port ID
 * Output:
 *      pSignalDetect   - Signal detect
 *      pSync           - Sync
 *      pLink           - Link
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port ID.
 * Note:
 *      The API can reset Serdes
 */
rtk_api_ret_t dal_rtl8367d_port_sgmiiLinkStatus_get(rtk_port_t port, rtk_data_t *pSignalDetect, rtk_data_t *pSync, rtk_port_linkStatus_t *pLink)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regValue;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    if(rtk_switch_isSgmiiPort(port) != RT_ERR_OK)
        return RT_ERR_PORT_ID;

    if(NULL == pSignalDetect)
        return RT_ERR_NULL_POINTER;

    if(NULL == pSync)
        return RT_ERR_NULL_POINTER;

    if(NULL == pLink)
        return RT_ERR_NULL_POINTER;

    switch (port)
    {
        case EXT_PORT0:
            if ((retVal = rtl8367d_setAsicReg(0x6601, 0x003D)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367d_setAsicReg(0x6600, 0x008D)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367d_getAsicReg(0x6602, &regValue)) != RT_ERR_OK)
                return retVal;
            break;
        case EXT_PORT1:
            if ((retVal = rtl8367d_setAsicReg(0x6601, 0x003D)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367d_setAsicReg(0x6600, 0x008F)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367d_getAsicReg(0x6602, &regValue)) != RT_ERR_OK)
                return retVal;
            break;
        default:
            return RT_ERR_PORT_ID;
    }

    *pSignalDetect = (regValue & 0x0100) ? 1 : 0;
    *pSync = (regValue & 0x0001) ? 1 : 0;
    *pLink = (regValue & 0x0010) ? 1 : 0;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_sgmiiNway_set
 * Description:
 *      Configure SGMII/HSGMII port Nway state
 * Input:
 *      port        - Port ID
 *      state       - Nway state
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port ID.
 * Note:
 *      The API configure SGMII/HSGMII port Nway state
 */
rtk_api_ret_t dal_rtl8367d_port_sgmiiNway_set(rtk_port_t port, rtk_enable_t state)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regValue;
    rtk_uint32 serdesAddr;

     /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    if(rtk_switch_isSgmiiPort(port) != RT_ERR_OK)
        return RT_ERR_PORT_ID;

    if(state >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    switch (port)
    {
        case EXT_PORT0:
            serdesAddr = 0x0D;
            break;
        case EXT_PORT1:
            serdesAddr = 0x0F;
            break;
        default:
            return RT_ERR_PORT_ID;
    }

    if ((retVal = rtl8367d_setAsicReg(0x6601, 0x0002)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367d_setAsicReg(0x6600, 0x0080 | serdesAddr)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367d_getAsicReg(0x6602, &regValue)) != RT_ERR_OK)
        return retVal;

    if(state)
        regValue |= 0x0200;
    else
        regValue &= ~0x0200;

    regValue |= 0x0100;

    if ((retVal = rtl8367d_setAsicReg(0x6602, regValue)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367d_setAsicReg(0x6601, 0x0002)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367d_setAsicReg(0x6600, 0x00C0 | serdesAddr)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_sgmiiNway_get
 * Description:
 *      Get SGMII/HSGMII port Nway state
 * Input:
 *      port        - Port ID
 * Output:
 *      pState      - Nway state
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port ID.
 * Note:
 *      The API can get SGMII/HSGMII port Nway state
 */
rtk_api_ret_t dal_rtl8367d_port_sgmiiNway_get(rtk_port_t port, rtk_enable_t *pState)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regValue;
    rtk_uint32 serdesAddr;

     /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    if(rtk_switch_isSgmiiPort(port) != RT_ERR_OK)
        return RT_ERR_PORT_ID;

    if(NULL == pState)
        return RT_ERR_NULL_POINTER;

    switch (port)
    {
        case EXT_PORT0:
            serdesAddr = 0x0D;
            break;
        case EXT_PORT1:
            serdesAddr = 0x0F;
            break;
        default:
            return RT_ERR_PORT_ID;
    }

    if ((retVal = rtl8367d_setAsicReg(0x6601, 0x0002)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367d_setAsicReg(0x6600, 0x0080 | serdesAddr)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367d_getAsicReg(0x6602, &regValue)) != RT_ERR_OK)
        return retVal;

    if((regValue & 0x0300) == 0x0300)
        *pState = ENABLED;
    else if((regValue & 0x0300) == 0x0000)
        *pState = ENABLED;
    else
        *pState = DISABLED;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_autoDos_set
 * Description:
 *      Set Auto Dos state
 * Input:
 *      type        - Auto DoS type
 *      state       - 1: Eanble(Drop), 0: Disable(Forward)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 * Note:
 *      The API can set Auto Dos state
 */
rtk_api_ret_t dal_rtl8367d_port_autoDos_set(rtk_port_autoDosType_t type, rtk_enable_t state)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (type >= AUTODOS_END)
        return RT_ERR_INPUT;

    if (state >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_DOS_CFG, RTL8367D_DROP_DAEQSA_OFFSET + type, (state == ENABLED) ? 1 : 0)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_port_autoDos_get
 * Description:
 *      Get Auto Dos state
 * Input:
 *      type        - Auto DoS type
 * Output:
 *      pState      - 1: Eanble(Drop), 0: Disable(Forward)
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_NULL_POINTER         - Null Pointer
 * Note:
 *      The API can get Auto Dos state
 */
rtk_api_ret_t dal_rtl8367d_port_autoDos_get(rtk_port_autoDosType_t type, rtk_enable_t *pState)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (type >= AUTODOS_END)
        return RT_ERR_INPUT;

    if (pState == NULL)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367d_getAsicRegBit(RTL8367D_REG_DOS_CFG, RTL8367D_DROP_DAEQSA_OFFSET + type, &regData)) != RT_ERR_OK)
        return retVal;

    *pState = (regData == 1) ? ENABLED : DISABLED;
    return RT_ERR_OK;
}

