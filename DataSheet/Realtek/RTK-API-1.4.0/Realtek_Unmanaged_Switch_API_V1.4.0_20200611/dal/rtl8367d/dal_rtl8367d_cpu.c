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
 * Feature : Here is a list of all functions and variables in CPU module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal/rtl8367d/dal_rtl8367d_cpu.h>

#include <string.h>

#include <dal/rtl8367d/rtl8367d_asicdrv.h>

/* Function Name:
 *      dal_rtl8367d_cpu_enable_set
 * Description:
 *      Set CPU port function enable/disable.
 * Input:
 *      enable - CPU port function enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can set CPU port function enable/disable.
 */
rtk_api_ret_t dal_rtl8367d_cpu_enable_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_CPU_CTRL, RTL8367D_CPU_EN_OFFSET, enable)) != RT_ERR_OK)
        return retVal;

    if (DISABLED == enable)
    {
        if ((retVal = rtl8367d_setAsicReg(RTL8367D_REG_CPU_PORT_MASK, 0)) != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8367d_cpu_enable_get
 * Description:
 *      Get CPU port and its setting.
 * Input:
 *      None
 * Output:
 *      pEnable - CPU port function enable
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_L2_NO_CPU_PORT   - CPU port is not exist
 * Note:
 *      The API can get CPU port function enable/disable.
 */
rtk_api_ret_t dal_rtl8367d_cpu_enable_get(rtk_enable_t *pEnable)

{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367d_getAsicRegBit(RTL8367D_REG_CPU_CTRL, RTL8367D_CPU_EN_OFFSET, pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_cpu_tagPort_set
 * Description:
 *      Set CPU port and CPU tag insert mode.
 * Input:
 *      port - Port id.
 *      mode - CPU tag insert for packets egress from CPU port.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can set CPU port and inserting proprietary CPU tag mode (Length/Type 0x8899)
 *      to the frame that transmitting to CPU port.
 *      The inset cpu tag mode is as following:
 *      - CPU_INSERT_TO_ALL
 *      - CPU_INSERT_TO_TRAPPING
 *      - CPU_INSERT_TO_NONE
 */
rtk_api_ret_t dal_rtl8367d_cpu_tagPort_set(rtk_port_t port, rtk_cpu_insert_t mode)
{
    rtk_api_ret_t retVal;
    rtk_uint32 phyPort;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

    if (mode >= CPU_INSERT_END)
        return RT_ERR_INPUT;

    phyPort = rtk_switch_port_L2P_get(port);
    if (phyPort == UNDEFINE_PHY_PORT)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8367d_setAsicReg(RTL8367D_REG_CPU_PORT_MASK, (1 << phyPort))) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_CPU_CTRL, RTL8367D_CPU_TRAP_PORT_MASK, (phyPort & 7))) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_CPU_CTRL, RTL8367D_CPU_INSERTMODE_MASK, mode)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_cpu_tagPort_get
 * Description:
 *      Get CPU port and CPU tag insert mode.
 * Input:
 *      None
 * Output:
 *      pPort - Port id.
 *      pMode - CPU tag insert for packets egress from CPU port, 0:all insert 1:Only for trapped packets 2:no insert.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_L2_NO_CPU_PORT   - CPU port is not exist
 * Note:
 *      The API can get configured CPU port and its setting.
 *      The inset cpu tag mode is as following:
 *      - CPU_INSERT_TO_ALL
 *      - CPU_INSERT_TO_TRAPPING
 *      - CPU_INSERT_TO_NONE
 */
rtk_api_ret_t dal_rtl8367d_cpu_tagPort_get(rtk_port_t *pPort, rtk_cpu_insert_t *pMode)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmsk, port;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pPort)
        return RT_ERR_NULL_POINTER;

    if(NULL == pMode)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367d_getAsicReg(RTL8367D_REG_CPU_PORT_MASK, &pmsk)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367d_getAsicRegBits(RTL8367D_REG_CPU_CTRL, RTL8367D_CPU_TRAP_PORT_MASK, &port)) != RT_ERR_OK)
        return retVal;

    *pPort = rtk_switch_port_P2L_get(port);

    if ((retVal = rtl8367d_getAsicRegBits(RTL8367D_REG_CPU_CTRL, RTL8367D_CPU_INSERTMODE_MASK, pMode)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_cpu_awarePort_set
 * Description:
 *      Set CPU aware port mask.
 * Input:
 *      portmask - Port mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK      - Invalid port mask.
 * Note:
 *      The API can set configured CPU aware port mask.
 */
rtk_api_ret_t dal_rtl8367d_cpu_awarePort_set(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 phyMbrPmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Valid port mask */
    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    /* Check port mask valid */
    RTK_CHK_PORTMASK_VALID(pPortmask);

    if(rtk_switch_portmask_L2P_get(pPortmask, &phyMbrPmask) != RT_ERR_OK)
        return RT_ERR_FAILED;

    if ((retVal = rtl8367d_setAsicReg(RTL8367D_REG_CPU_PORT_MASK, phyMbrPmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_cpu_awarePort_get
 * Description:
 *      Get CPU aware port mask.
 * Input:
 *      None
 * Output:
 *      pPortmask - Port mask.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 * Note:
 *      The API can get configured CPU aware port mask.
 */
rtk_api_ret_t dal_rtl8367d_cpu_awarePort_get(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmsk;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367d_getAsicReg(RTL8367D_REG_CPU_PORT_MASK, &pmsk)) != RT_ERR_OK)
        return retVal;

    if(rtk_switch_portmask_P2L_get(pmsk, pPortmask) != RT_ERR_OK)
        return RT_ERR_FAILED;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_cpu_tagPosition_set
 * Description:
 *      Set CPU tag position.
 * Input:
 *      position - CPU tag position.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT      - Invalid input.
 * Note:
 *      The API can set CPU tag position.
 */
rtk_api_ret_t dal_rtl8367d_cpu_tagPosition_set(rtk_cpu_position_t position)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (position >= CPU_POS_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_CPU_CTRL, RTL8367D_CPU_TAG_POSITION_OFFSET, position)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_cpu_tagPosition_get
 * Description:
 *      Get CPU tag position.
 * Input:
 *      None
 * Output:
 *      pPosition - CPU tag position.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT      - Invalid input.
 * Note:
 *      The API can get CPU tag position.
 */
rtk_api_ret_t dal_rtl8367d_cpu_tagPosition_get(rtk_cpu_position_t *pPosition)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pPosition)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367d_getAsicRegBit(RTL8367D_REG_CPU_CTRL, RTL8367D_CPU_TAG_POSITION_OFFSET, pPosition)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_cpu_tagLength_set
 * Description:
 *      Set CPU tag length.
 * Input:
 *      length - CPU tag length.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT      - Invalid input.
 * Note:
 *      The API can set CPU tag length.
 */
rtk_api_ret_t dal_rtl8367d_cpu_tagLength_set(rtk_cpu_tag_length_t length)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    switch (length)
    {
        case CPU_LEN_8BYTES:
            if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_CPU_CTRL, RTL8367D_CPU_TAG_FORMAT_OFFSET, 0)) != RT_ERR_OK)
                return retVal;
            if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_CPU_CTRL, RTL8367D_CPU_TAG_FORMAT_PRI_OFFSET, 0)) != RT_ERR_OK)
                return retVal;
            break;
        case CPU_LEN_4BYTES:
            if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_CPU_CTRL, RTL8367D_CPU_TAG_FORMAT_OFFSET, 1)) != RT_ERR_OK)
                return retVal;
            if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_CPU_CTRL, RTL8367D_CPU_TAG_FORMAT_PRI_OFFSET, 0)) != RT_ERR_OK)
                return retVal;
            break;
        case CPU_LEN_4BYTES_PRIORITY:
            if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_CPU_CTRL, RTL8367D_CPU_TAG_FORMAT_OFFSET, 1)) != RT_ERR_OK)
                return retVal;
            if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_CPU_CTRL, RTL8367D_CPU_TAG_FORMAT_PRI_OFFSET, 1)) != RT_ERR_OK)
                return retVal;
            break;
        default:
            return RT_ERR_INPUT;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_cpu_tagLength_get
 * Description:
 *      Get CPU tag length.
 * Input:
 *      None
 * Output:
 *      pLength - CPU tag length.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT      - Invalid input.
 * Note:
 *      The API can get CPU tag length.
 */
rtk_api_ret_t dal_rtl8367d_cpu_tagLength_get(rtk_cpu_tag_length_t *pLength)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;
    rtk_uint32 regData2;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pLength)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367d_getAsicRegBit(RTL8367D_REG_CPU_CTRL, RTL8367D_CPU_TAG_FORMAT_OFFSET, &regData)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367d_getAsicRegBit(RTL8367D_REG_CPU_CTRL, RTL8367D_CPU_TAG_FORMAT_PRI_OFFSET, &regData2)) != RT_ERR_OK)
        return retVal;

    if (regData == 0)
        *pLength = CPU_LEN_8BYTES;
    else
    {
        if (regData2 == 0)
            *pLength = CPU_LEN_4BYTES;
        else
            *pLength = CPU_LEN_4BYTES_PRIORITY;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_cpu_priRemap_set
 * Description:
 *      Configure CPU priorities mapping to internal absolute priority.
 * Input:
 *      int_pri     - internal priority value.
 *      new_pri    - new internal priority value.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_VLAN_PRIORITY    - Invalid 1p priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      Priority of CPU tag assignment for internal asic priority, and it is used for queue usage and packet scheduling.
 */
rtk_api_ret_t dal_rtl8367d_cpu_priRemap_set(rtk_pri_t int_pri, rtk_pri_t new_pri)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (new_pri > RTL8367D_PRIMAX || int_pri > RTL8367D_PRIMAX)
        return  RT_ERR_VLAN_PRIORITY;

    if ((retVal = rtl8367d_setAsicRegBits(RTL8367D_REG_QOS_PRIORITY_REMAPPING_IN_CPU_CTRL0 + (int_pri >> 2), RTL8367D_QOS_PRIORITY_REMAPPING_IN_CPU_CTRL0_PRIORITY0_MASK << ((int_pri & 0x3) << 2), new_pri)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_cpu_priRemap_get
 * Description:
 *      Configure CPU priorities mapping to internal absolute priority.
 * Input:
 *      int_pri     - internal priority value.
 * Output:
 *      pNew_pri    - new internal priority value.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_VLAN_PRIORITY    - Invalid 1p priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      Priority of CPU tag assignment for internal asic priority, and it is used for queue usage and packet scheduling.
 */
rtk_api_ret_t dal_rtl8367d_cpu_priRemap_get(rtk_pri_t int_pri, rtk_pri_t *pNew_pri)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pNew_pri)
        return RT_ERR_NULL_POINTER;

    if (int_pri > RTL8367D_PRIMAX)
        return  RT_ERR_QOS_INT_PRIORITY;

    if ((retVal = rtl8367d_getAsicRegBits(RTL8367D_REG_QOS_PRIORITY_REMAPPING_IN_CPU_CTRL0 + (int_pri >> 2), RTL8367D_QOS_PRIORITY_REMAPPING_IN_CPU_CTRL0_PRIORITY0_MASK << ((int_pri & 0x3) << 2), pNew_pri)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_cpu_acceptLength_set
 * Description:
 *      Set CPU accept  length.
 * Input:
 *      length - CPU tag length.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT      - Invalid input.
 * Note:
 *      The API can set CPU accept length.
 */
rtk_api_ret_t dal_rtl8367d_cpu_acceptLength_set(rtk_cpu_rx_length_t length)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (length >= CPU_RX_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367d_setAsicRegBit(RTL8367D_REG_CPU_CTRL, RTL8367D_CPU_TAG_RXBYTECOUNT_OFFSET, length)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367d_cpu_acceptLength_get
 * Description:
 *      Get CPU accept length.
 * Input:
 *      None
 * Output:
 *      pLength - CPU tag length.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT      - Invalid input.
 * Note:
 *      The API can get CPU accept length.
 */
rtk_api_ret_t dal_rtl8367d_cpu_acceptLength_get(rtk_cpu_rx_length_t *pLength)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pLength)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367d_getAsicRegBit(RTL8367D_REG_CPU_CTRL, RTL8367D_CPU_TAG_RXBYTECOUNT_OFFSET, pLength)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

