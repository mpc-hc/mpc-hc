void DumpBuffer(struct Buffer *b, unsigned char * buf, int len)
{
#define NB_BYTE 16 /* number of bytes displayed per line */

	char str[NB_BYTE*3 + 1];
	int i,j;

	for (i=0;i<len;i+=NB_BYTE)
	{
		char * p = str;

		for (j=i;j<len&&j<i+NB_BYTE;j++)
		{
			char c;

			*p++ = ' ';
			c = (buf[j] >> 4) & 0xf;
			*p++ = (c<10) ? c+'0' : c-10+'a';
			c = buf[j] & 0xf;
			*p++ = (c<10) ? c+'0' : c-10+'a';
		}
		*p = 0;

		BufferPrintf(b,"    %x:%s\n",i,str);
	}
}

/*
	Dump a buffer. Its address is either pBuffer or pMdl.
	It dumps for uBufferSize bytes starting at
	pBuffer (or pMDL) + uBufferOffset. This later is optionnal (currently)
*/

void DumpTransferBuffer(struct Buffer *b,
						PUCHAR pBuffer, PMDL pMdl, ULONG uBufferSize,
						BOOLEAN bPrintHeader,ULONG uBufferOffset = 0)
{
	if(bPrintHeader)
	{
		BufferPrintf(b,"  TransferBufferLength = %x\n", uBufferSize);
		BufferPrintf(b,"  TransferBuffer       = %x\n", pBuffer);
		BufferPrintf(b,"  TransferBufferMDL    = %x\n", pMdl);
	}
	else
	{
		if(pBuffer)
		{
			if(pMdl)
			{
				//LogPrintf("??? weird transfer buffer, both MDL and flat specified. Ignoring MDL\n"));
			}
			DumpBuffer(b,pBuffer+uBufferOffset,uBufferSize);
		}
		else if(pMdl)
		{
			PUCHAR pMDLBuf = (PUCHAR)MmGetSystemAddressForMdl(pMdl);
			if(pMDLBuf)
				DumpBuffer(b,pMDLBuf+uBufferOffset,uBufferSize);
			else
			{
				BufferPrintf(b,"XXXXX ERROR: can't map MDL!\n");
			}
		}
		else
		{
			BufferPrintf(b,"\n    no data supplied\n");
		}
	}
}

void DumpGetStatusRequest(struct Buffer *b,
						  struct _URB_CONTROL_GET_STATUS_REQUEST *pGetStatusRequest,
						  BOOLEAN bReturnedFromHCD)
{
	DumpTransferBuffer(b,(PUCHAR)pGetStatusRequest->TransferBuffer, pGetStatusRequest->TransferBufferMDL, pGetStatusRequest->TransferBufferLength, TRUE);
	if(pGetStatusRequest->TransferBufferLength != 1)
		BufferPrintf(b,"  *** error - TransferBufferLength should be 1!\n");
	if(bReturnedFromHCD)
	{
		DumpTransferBuffer(b,(PUCHAR)pGetStatusRequest->TransferBuffer, pGetStatusRequest->TransferBufferMDL, pGetStatusRequest->TransferBufferLength, FALSE);
	}

	BufferPrintf(b,"  Index                = %x\n", pGetStatusRequest->Index);
	
	if(pGetStatusRequest->UrbLink)
	{
		BufferPrintf(b,"---> Linked URB:\n");
		DumpURB(b,pGetStatusRequest->UrbLink, bReturnedFromHCD);
		BufferPrintf(b,"---< Linked URB\n");
	}
}

void DumpFeatureRequest(struct Buffer *b,
						struct _URB_CONTROL_FEATURE_REQUEST *pFeatureRequest,
						BOOLEAN bReadFromDevice, BOOLEAN bReturnedFromHCD)
{
	BufferPrintf(b,"  FeatureSelector = %x\n", pFeatureRequest->FeatureSelector);
	BufferPrintf(b,"  Index           = %x\n", pFeatureRequest->Index);
	if(pFeatureRequest->UrbLink)
	{
		BufferPrintf(b,"---> Linked URB:\n");
		DumpURB(b,pFeatureRequest->UrbLink, bReturnedFromHCD);
		BufferPrintf(b,"---< Linked URB\n");
	}
}

void DumpDescriptorRequest(struct Buffer *b,
						   struct _URB_CONTROL_DESCRIPTOR_REQUEST *pDescriptorRequest,
						   BOOLEAN bReadFromDevice, BOOLEAN bReturnedFromHCD)
{
	DumpTransferBuffer(b,(PUCHAR)pDescriptorRequest->TransferBuffer, pDescriptorRequest->TransferBufferMDL, pDescriptorRequest->TransferBufferLength, TRUE);
	if(((!bReadFromDevice) && (!bReturnedFromHCD)) || (bReadFromDevice && bReturnedFromHCD))
	{
		DumpTransferBuffer(b,(PUCHAR)pDescriptorRequest->TransferBuffer, pDescriptorRequest->TransferBufferMDL, pDescriptorRequest->TransferBufferLength, FALSE);
	}

	BufferPrintf(b,"  Index                = %x\n", pDescriptorRequest->Index);
	BufferPrintf(b,"  DescriptorType       = %x (%s)\n", pDescriptorRequest->DescriptorType,
		pDescriptorRequest->DescriptorType == USB_DEVICE_DESCRIPTOR_TYPE ? "USB_DEVICE_DESCRIPTOR_TYPE" :
		pDescriptorRequest->DescriptorType == USB_CONFIGURATION_DESCRIPTOR_TYPE ? "USB_CONFIGURATION_DESCRIPTOR_TYPE" :
		pDescriptorRequest->DescriptorType == USB_STRING_DESCRIPTOR_TYPE ? "USB_STRING_DESCRIPTOR_TYPE" : "<illegal descriptor type!>");
	BufferPrintf(b,"  LanguageId           = %x\n", pDescriptorRequest->LanguageId);
	
	if(pDescriptorRequest->UrbLink)
	{
		BufferPrintf(b,"---> Linked URB:\n");
		DumpURB(b,pDescriptorRequest->UrbLink, bReturnedFromHCD);
		BufferPrintf(b,"---< Linked URB\n");
	}
}

void DumpVendorOrClassRequest(struct Buffer *b,
							  struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *pFunctionClassInterface, BOOLEAN bReturnedFromHCD)
{
	BOOLEAN bReadFromDevice = (BOOLEAN)(pFunctionClassInterface->TransferFlags & USBD_TRANSFER_DIRECTION_IN);
	BufferPrintf(b,"  TransferFlags          = %x (%s, %sUSBD_SHORT_TRANSFER_OK)\n", pFunctionClassInterface->TransferFlags,
		bReadFromDevice ? "USBD_TRANSFER_DIRECTION_IN" : "USBD_TRANSFER_DIRECTION_OUT",
		pFunctionClassInterface->TransferFlags & USBD_SHORT_TRANSFER_OK ? "":"~");

	DumpTransferBuffer(b,(PUCHAR)pFunctionClassInterface->TransferBuffer, pFunctionClassInterface->TransferBufferMDL, pFunctionClassInterface->TransferBufferLength, TRUE);
	if(((!bReadFromDevice) && (!bReturnedFromHCD)) || (bReadFromDevice && bReturnedFromHCD))
	{
		DumpTransferBuffer(b,(PUCHAR)pFunctionClassInterface->TransferBuffer, pFunctionClassInterface->TransferBufferMDL, pFunctionClassInterface->TransferBufferLength, FALSE);
	}

	BufferPrintf(b,"  UrbLink                 = %x\n", pFunctionClassInterface->UrbLink);
	BufferPrintf(b,"  RequestTypeReservedBits = %x\n", pFunctionClassInterface->RequestTypeReservedBits);
	BufferPrintf(b,"  Request                 = %x\n", pFunctionClassInterface->Request);
	BufferPrintf(b,"  Value                   = %x\n", pFunctionClassInterface->Value);
	BufferPrintf(b,"  Index                   = %x\n", pFunctionClassInterface->Index);
	if(pFunctionClassInterface->UrbLink)
	{
		BufferPrintf(b,"---> Linked URB:\n");
		DumpURB(b,pFunctionClassInterface->UrbLink, bReturnedFromHCD);
		BufferPrintf(b,"---< Linked URB\n");
	}
}

void DumpPipeHandle(struct Buffer *b,const char *s,
					USBD_PIPE_HANDLE inPipeHandle)
{
	unsigned char ep;

	// search for the matching endpoint

	if (GetEndpointInfo(inPipeHandle,&ep))
		BufferPrintf(b,"%s = %p [endpoint 0x%x]\n",s,inPipeHandle,ep);
	else
		BufferPrintf(b,"%s = %p\n",s,inPipeHandle);
}

void DumpStackLocation(PIO_STACK_LOCATION stack)
{
	if (stack == NULL)
		return ;

	LogPrintf("    MajorFunction=%d, MinorFunction=%d\n",
		stack->MajorFunction,stack->MinorFunction);
	LogPrintf("    DeviceObject=%p\n",stack->DeviceObject);
	LogPrintf("    CompletionRoutine=%p Context=%p\n",
		stack->CompletionRoutine,stack->Context);

}

void DumpIrp(PIRP Irp)
{
	CHAR i;

	LogPrintf("Dumping IRP %p\n",Irp);
	if (Irp==NULL)
		return ;

	LogPrintf("  Type=%d, Size=%d\n",Irp->Type,Irp->Size);
	LogPrintf("  StackCount=%d, CurrentLocation=%d\n",Irp->StackCount,
		Irp->CurrentLocation);
	for (i=0;i<Irp->StackCount;i++)
	{
		PIO_STACK_LOCATION stack = (PIO_STACK_LOCATION) (Irp+1) + i;
		LogPrintf("  [%d] MajorFunction=%d, MinorFunction=%d, DeviceObject=%p\n",
			i,stack->MajorFunction,stack->MinorFunction,stack->DeviceObject);
		LogPrintf("      Arg1=%p, Arg2=%p, Arg3=%p, Arg4=%p\n",
			stack->Parameters.Others.Argument1,stack->Parameters.Others.Argument2,
			stack->Parameters.Others.Argument3,stack->Parameters.Others.Argument4);
		LogPrintf("      CompletionRoutine=%p Context=%p\n",
			stack->CompletionRoutine,stack->Context);
	}
}

void DumpDriverObject(PDRIVER_OBJECT p)
{
	int i;

	LogPrintf("PN31Snoop - DumpDriverObject : p = %p\n",p);
	LogPrintf("  Type = %d\n",p->Type);
	LogPrintf("  Size = %d\n",p->Size);
	LogPrintf("  DeviceObject = %p\n",p->DeviceObject);
	LogPrintf("  Flags = 0x%x\n",p->Flags);
	LogPrintf("  DriverStart = %p\n",p->DriverStart);
	LogPrintf("  DriverSize = %d\n",p->DriverSize);
	LogPrintf("  DriverSection = %p\n",p->DriverSection);
	LogPrintf("  DriverExtension = %p\n",p->DriverExtension);
	LogPrintf("  DriverExtension->AddDevice = %p\n",p->DriverExtension->AddDevice);
	LogPrintf("  FastIoDispatch = %p\n",p->FastIoDispatch);
	LogPrintf("  DriverInit = %p\n",p->DriverInit);
	LogPrintf("  DriverStartIo = %p\n",p->DriverStartIo);
	LogPrintf("  DriverUnload = %p\n",p->DriverUnload);
	for (i=0;i<IRP_MJ_MAXIMUM_FUNCTION + 1;i++)
		LogPrintf("  MajorFunction[%d] = %p\n",i,p->MajorFunction[i]);
}

void DumpDeviceObject(PDEVICE_OBJECT p)
{
	PDEVICE_EXTENSION pdx;

	LogPrintf("PN31Snoop - DumpDeviceObject : p = %p\n",p);
	LogPrintf("  DriverObject = %p\n",p->DriverObject);
	LogPrintf("  NextDevice = %p\n",p->NextDevice);
	LogPrintf("  AttachedDevice = %p\n",p->AttachedDevice);
	LogPrintf("  StackSize=%d\n",p->StackSize);
	LogPrintf("  CurrentIrp = %p\n",p->CurrentIrp);
	LogPrintf("  DeviceObjectExtension = %p\n",p->DeviceObjectExtension);

	pdx = (PDEVICE_EXTENSION)p->DeviceObjectExtension;

	LogPrintf("   ->DeviceObject=%p\n",pdx->DeviceObject);
	LogPrintf("   ->LowerDeviceObject=%p\n",pdx->LowerDeviceObject);
	LogPrintf("   ->Pdo=%p\n",pdx->Pdo);
}

void DumpContext(PCONTEXT Context)
{
	LogPrintf("DumpContext : Context=%p\n",Context);
	LogPrintf("  CompletionRoutine=%p, Context=%p, Control=%x\n",
		Context->CompletionRoutine,Context->Context,Context->Control);
	LogPrintf("  pUrb=%p, uSequenceNumber=%d, Stack=%p\n",
		Context->pUrb,Context->uSequenceNumber,Context->Stack);
}


void DumpURB(struct Buffer *b, PURB pUrb, BOOLEAN bReturnedFromHCD)
{
	USHORT wFunction, wLength;
	USBD_STATUS lUsbdStatus;

	if (pUrb == NULL)
	{
		BufferPrintf(b,"PN31Snoop - URB == NULL ???\n");
		return;
	}

	if (pUrb->UrbHeader.Length < sizeof(pUrb->UrbHeader))
	{
		BufferPrintf(b,"PN31Snoop - incorrect UrbHeader.Length=%d, should be at least %d\n",
			pUrb->UrbHeader.Length,sizeof(pUrb->UrbHeader));
		return ;
	}

	wFunction = pUrb->UrbHeader.Function;
	wLength = pUrb->UrbHeader.Length;
	lUsbdStatus = pUrb->UrbHeader.Status;

	/* Status values are defined in <usbdi.h> as USBD_STATUS_XXX */
//	LogPrintf("  Header.Length = %d\n",          pUrb->UrbHeader.Length));
//	LogPrintf("  Header.Function = 0x%x\n",      pUrb->UrbHeader.Function));
//	LogPrintf("  Header.Status = 0x%x\n",        pUrb->UrbHeader.Status));
//	LogPrintf("  Header.UsbdDeviceHandle = %p\n",pUrb->UrbHeader.UsbdDeviceHandle));
//	LogPrintf("  Header.UsbdFlags = 0x%x\n",     pUrb->UrbHeader.UsbdFlags));

	switch(wFunction)
	{
	case URB_FUNCTION_SELECT_CONFIGURATION:
		{

			/* _URB_SELECT_CONFIGURATION is as follows :

			- a first block of 16 bytes : struct _URB_HEADER Hdr
			- a pointer (4 bytes) : PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor
			      this can be a NULL pointer, in which case the array of USBD_INTERFACE_INFORMATION
				  is empty.
			- a handle (4 bytes) : USBD_CONFIGURATION_HANDLE ConfigurationHandle
			- an array of USBD_INTERFACE_INFORMATION, whose number are
			    ConfigurationDescriptor.bNumInterfaces

			each USBD_INTERFACE_INFORMATION contains fixed information (16 bytes), followed
			  by an array of USB_PIPE_INFORMATION (20 bytes) whose number is NumberOfPipes.
			*/

#define URB_SELECT_CONFIGURATION_SIZE 24

			struct _URB_SELECT_CONFIGURATION *pSelectConfiguration = (struct _URB_SELECT_CONFIGURATION*) pUrb;
			BufferPrintf(b,"-- URB_FUNCTION_SELECT_CONFIGURATION:\n");
			if(pSelectConfiguration->Hdr.Length < URB_SELECT_CONFIGURATION_SIZE)
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n",
					pSelectConfiguration->Hdr.Length,URB_SELECT_CONFIGURATION_SIZE);
				return ;
			}

			PUSB_CONFIGURATION_DESCRIPTOR pCD = pSelectConfiguration->ConfigurationDescriptor;
			BufferPrintf(b,"  ConfigurationDescriptor = 0x%x %s\n",pCD,pCD ? "(configure)":"(unconfigure)");
			if (pCD == NULL)
				break;

			BufferPrintf(b,"  ConfigurationDescriptor : bLength             = %d\n", pCD->bLength);
			BufferPrintf(b,"  ConfigurationDescriptor : bDescriptorType     = 0x%x\n", pCD->bDescriptorType);
			BufferPrintf(b,"  ConfigurationDescriptor : wTotalLength        = 0x%x\n", pCD->wTotalLength);
			BufferPrintf(b,"  ConfigurationDescriptor : bNumInterfaces      = 0x%x\n", pCD->bNumInterfaces);
			BufferPrintf(b,"  ConfigurationDescriptor : bConfigurationValue = 0x%x\n", pCD->bConfigurationValue);
			BufferPrintf(b,"  ConfigurationDescriptor : iConfiguration      = 0x%x\n", pCD->iConfiguration);
			BufferPrintf(b,"  ConfigurationDescriptor : bmAttributes        = 0x%x\n", pCD->bmAttributes);
			BufferPrintf(b,"  ConfigurationDescriptor : MaxPower            = 0x%x\n", pCD->MaxPower);
			BufferPrintf(b,"  ConfigurationHandle     = 0x%x\n", pSelectConfiguration->ConfigurationHandle);
			
			ULONG uNumInterfaces = pCD->bNumInterfaces;

			if(uNumInterfaces > 0xff)
			{
				BufferPrintf(b,"XXXXXX ERROR: uNumInterfaces is too large (%d), resetting to 1\n", uNumInterfaces);
				uNumInterfaces = 1;
			}
			
			
			PUSBD_INTERFACE_INFORMATION pInterface = &pSelectConfiguration->Interface;
			for(ULONG i = 0; i < uNumInterfaces; i++)
			{
				BufferPrintf(b,"  Interface[%d]: Length            = %d\n", i, pInterface->Length);
				BufferPrintf(b,"  Interface[%d]: InterfaceNumber   = %d\n", i, pInterface->InterfaceNumber);
				BufferPrintf(b,"  Interface[%d]: AlternateSetting  = %d\n", i, pInterface->AlternateSetting);
				if(bReturnedFromHCD)
				{
					ULONG uNumPipes;
					BufferPrintf(b,"  Interface[%d]: Class             = 0x%x\n", i, pInterface->Class);
					BufferPrintf(b,"  Interface[%d]: SubClass          = 0x%x\n", i, pInterface->SubClass);
					BufferPrintf(b,"  Interface[%d]: Protocol          = 0x%x\n", i, pInterface->Protocol);
					BufferPrintf(b,"  Interface[%d]: InterfaceHandle   = 0x%x\n", i, pInterface->InterfaceHandle);
					BufferPrintf(b,"  Interface[%d]: NumberOfPipes     = %d\n", i, pInterface->NumberOfPipes);
					
					uNumPipes = pInterface->NumberOfPipes;
					if(uNumPipes > 0x1f)
					{
						BufferPrintf(b,"XXXXXX ERROR: uNumPipes is too large (%d), resetting to 1\n", uNumPipes);
						uNumPipes = 1;
					}
					for(ULONG p = 0; p< uNumPipes; p++)
					{
						BufferPrintf(b,"  Interface[%d]: Pipes[%d] : MaximumPacketSize = 0x%x\n", i, p, pInterface->Pipes[p].MaximumPacketSize);
						BufferPrintf(b,"  Interface[%d]: Pipes[%d] : EndpointAddress   = 0x%x\n", i, p, pInterface->Pipes[p].EndpointAddress);
						BufferPrintf(b,"  Interface[%d]: Pipes[%d] : Interval          = 0x%x\n", i, p, pInterface->Pipes[p].Interval);
						BufferPrintf(b,"  Interface[%d]: Pipes[%d] : PipeType          = 0x%x (%s)\n", i, p, pInterface->Pipes[p].PipeType,
							pInterface->Pipes[p].PipeType == UsbdPipeTypeControl ? "UsbdPipeTypeControl" :
						pInterface->Pipes[p].PipeType == UsbdPipeTypeIsochronous ? "UsbdPipeTypeIsochronous" :
						pInterface->Pipes[p].PipeType == UsbdPipeTypeBulk ? "UsbdPipeTypeBulk" :
						pInterface->Pipes[p].PipeType == UsbdPipeTypeInterrupt ? "UsbdPipeTypeInterrupt" : "!!! INVALID !!!");
						BufferPrintf(b,"  Interface[%d]: Pipes[%d] : PipeHandle        = 0x%p\n", i, p, pInterface->Pipes[p].PipeHandle);
						BufferPrintf(b,"  Interface[%d]: Pipes[%d] : MaxTransferSize   = 0x%x\n", i, p, pInterface->Pipes[p].MaximumTransferSize);
						BufferPrintf(b,"  Interface[%d]: Pipes[%d] : PipeFlags         = 0x%x\n", i, p, pInterface->Pipes[p].PipeFlags);

						AddEndpointInfo(pInterface->Pipes[p].PipeHandle,
							pInterface->Pipes[p].EndpointAddress);
					}
				}

				pInterface = (PUSBD_INTERFACE_INFORMATION) (((UCHAR*)pInterface) + pInterface->Length);
			}
		}
		break;

	case URB_FUNCTION_SELECT_INTERFACE:
		{
			struct _URB_SELECT_INTERFACE  *pSelectInterface = (struct _URB_SELECT_INTERFACE *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_SELECT_INTERFACE:\n");
			if(pSelectInterface->Hdr.Length < sizeof(struct _URB_SELECT_INTERFACE))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n",
					pSelectInterface->Hdr.Length, sizeof(struct _URB_SELECT_INTERFACE));
				return ;
			}
			BufferPrintf(b,"  ConfigurationHandle     = 0x%x\n", pSelectInterface->ConfigurationHandle);

			PUSBD_INTERFACE_INFORMATION pInterface = &pSelectInterface->Interface;
			
			BufferPrintf(b,"  Interface: Length            = %d\n", pInterface->Length);
			BufferPrintf(b,"  Interface: InterfaceNumber   = %d\n", pInterface->InterfaceNumber);
			BufferPrintf(b,"  Interface: AlternateSetting  = %d\n", pInterface->AlternateSetting);
			BufferPrintf(b,"  Interface: Class             = 0x%x\n", pInterface->Class);
			BufferPrintf(b,"  Interface: SubClass          = 0x%x\n", pInterface->SubClass);
			BufferPrintf(b,"  Interface: Protocol          = 0x%x\n", pInterface->Protocol);
			BufferPrintf(b,"  Interface: InterfaceHandle   = %p\n", pInterface->InterfaceHandle);
			BufferPrintf(b,"  Interface: NumberOfPipes     = %d\n", pInterface->NumberOfPipes);
			if(bReturnedFromHCD)
			{
				ULONG uNumPipes = pInterface->NumberOfPipes;
				if(uNumPipes > 0x1f)
				{
					BufferPrintf(b,"XXXXXX ERROR: uNumPipes is too large (%d), resetting to 1\n", uNumPipes);
					uNumPipes = 1;
				}
				for(ULONG p = 0; p< uNumPipes; p++)
				{
					BufferPrintf(b,"  Interface: Pipes[%d] : MaximumPacketSize = 0x%x\n", p, pInterface->Pipes[p].MaximumPacketSize);
					BufferPrintf(b,"  Interface: Pipes[%d] : EndpointAddress   = 0x%x\n", p, pInterface->Pipes[p].EndpointAddress);
					BufferPrintf(b,"  Interface: Pipes[%d] : Interval          = 0x%x\n", p, pInterface->Pipes[p].Interval);
					BufferPrintf(b,"  Interface: Pipes[%d] : PipeType          = 0x%x (%s)\n", p, pInterface->Pipes[p].PipeType,
						pInterface->Pipes[p].PipeType == UsbdPipeTypeControl ? "UsbdPipeTypeControl" :
					pInterface->Pipes[p].PipeType == UsbdPipeTypeIsochronous ? "UsbdPipeTypeIsochronous" :
					pInterface->Pipes[p].PipeType == UsbdPipeTypeBulk ? "UsbdPipeTypeBulk" :
					pInterface->Pipes[p].PipeType == UsbdPipeTypeInterrupt ? "UsbdPipeTypeInterrupt" : "!!! INVALID !!!");
					BufferPrintf(b,"  Interface: Pipes[%d] : PipeHandle        = 0x%p\n", p, pInterface->Pipes[p].PipeHandle);
					BufferPrintf(b,"  Interface: Pipes[%d] : MaxTransferSize   = 0x%x\n", p, pInterface->Pipes[p].MaximumTransferSize);
					BufferPrintf(b,"  Interface: Pipes[%d] : PipeFlags         = 0x%x\n", p, pInterface->Pipes[p].PipeFlags);

					AddEndpointInfo(pInterface->Pipes[p].PipeHandle,
						pInterface->Pipes[p].EndpointAddress);
				}
			}
		}
		break;

	case URB_FUNCTION_ABORT_PIPE: // tested [22/11/2001]
		{
			struct _URB_PIPE_REQUEST   *pAbortPipe = (struct _URB_PIPE_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_ABORT_PIPE:\n");
			if(pAbortPipe->Hdr.Length < sizeof(struct _URB_PIPE_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n",
					pAbortPipe->Hdr.Length, sizeof(struct _URB_PIPE_REQUEST));
				return ;
			}

			if(!bReturnedFromHCD)
				DumpPipeHandle(b,"  PipeHandle",pAbortPipe->PipeHandle);
		}
		break;

	case URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL: // untested
		{
			struct _URB_FRAME_LENGTH_CONTROL *pFrameLengthControl = (struct _URB_FRAME_LENGTH_CONTROL *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL:\n");
			if(pFrameLengthControl->Hdr.Length < sizeof(struct _URB_FRAME_LENGTH_CONTROL))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n",
					pFrameLengthControl->Hdr.Length, sizeof(struct _URB_FRAME_LENGTH_CONTROL));
				return ;
			}

			BufferPrintf(b,"  (no parameters)\n");
		}
		break;

	case URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL: // untested
		{
			struct _URB_FRAME_LENGTH_CONTROL *pFrameLengthControl = (struct _URB_FRAME_LENGTH_CONTROL *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL:\n");
			if(pFrameLengthControl->Hdr.Length < sizeof(struct _URB_FRAME_LENGTH_CONTROL))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n",
					pFrameLengthControl->Hdr.Length, sizeof(struct _URB_FRAME_LENGTH_CONTROL));
				return ;
			}

			BufferPrintf(b,"  (no parameters)\n");
		}
		break;

	case URB_FUNCTION_GET_FRAME_LENGTH: // untested
		{
			struct _URB_GET_FRAME_LENGTH   *pGetFrameLength = (struct _URB_GET_FRAME_LENGTH *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_GET_FRAME_LENGTH:\n");
			if(pGetFrameLength->Hdr.Length < sizeof(struct _URB_GET_FRAME_LENGTH))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n",
					pGetFrameLength->Hdr.Length, sizeof(struct _URB_GET_FRAME_LENGTH));
				return ;
			}

			if(bReturnedFromHCD)
			{
				BufferPrintf(b,"  FrameLength = %x\n", pGetFrameLength->FrameLength);
				BufferPrintf(b,"  FrameNumber = %x\n", pGetFrameLength->FrameNumber);
			}
		}
		break;

	case URB_FUNCTION_SET_FRAME_LENGTH: // untested
		{
			struct _URB_SET_FRAME_LENGTH   *pSetFrameLength = (struct _URB_SET_FRAME_LENGTH *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_SET_FRAME_LENGTH:\n");
			if(pSetFrameLength->Hdr.Length < sizeof(struct _URB_SET_FRAME_LENGTH))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n",
					pSetFrameLength->Hdr.Length, sizeof(struct _URB_SET_FRAME_LENGTH));
				return ;
			}

			if(!bReturnedFromHCD)
				BufferPrintf(b,"  FrameLengthDelta = %x\n", pSetFrameLength->FrameLengthDelta);
		}
		break;

	case URB_FUNCTION_GET_CURRENT_FRAME_NUMBER: // untested
		{
			struct _URB_GET_CURRENT_FRAME_NUMBER   *pGetCurrentFrameNumber = (struct _URB_GET_CURRENT_FRAME_NUMBER *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_GET_CURRENT_FRAME_NUMBER:\n");
			if(pGetCurrentFrameNumber->Hdr.Length < sizeof(struct _URB_GET_CURRENT_FRAME_NUMBER))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n",
					pGetCurrentFrameNumber->Hdr.Length, sizeof(struct _URB_GET_CURRENT_FRAME_NUMBER));
				return ;
			}

			if(bReturnedFromHCD)
				BufferPrintf(b,"  FrameNumber = %x\n", pGetCurrentFrameNumber->FrameNumber);
		}
		break;

	case URB_FUNCTION_CONTROL_TRANSFER: // tested [22/11/2001]
		{
			struct _URB_CONTROL_TRANSFER   *pControlTransfer = (struct _URB_CONTROL_TRANSFER *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_CONTROL_TRANSFER:\n");
			if(pControlTransfer->Hdr.Length < sizeof(struct _URB_CONTROL_TRANSFER))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n",
					pControlTransfer->Hdr.Length, sizeof(struct _URB_CONTROL_TRANSFER));
				return ;
			}

			BOOLEAN bReadFromDevice = (BOOLEAN)(pControlTransfer->TransferFlags & USBD_TRANSFER_DIRECTION_IN);
			DumpPipeHandle(b,"  PipeHandle          ",pControlTransfer->PipeHandle);
			BufferPrintf(b,"  TransferFlags        = %x (%s, %sUSBD_SHORT_TRANSFER_OK)\n", pControlTransfer->TransferFlags,
				bReadFromDevice ? "USBD_TRANSFER_DIRECTION_IN" : "USBD_TRANSFER_DIRECTION_OUT",
				pControlTransfer->TransferFlags & USBD_SHORT_TRANSFER_OK ? "":"~");
			DumpTransferBuffer(b,(PUCHAR)pControlTransfer->TransferBuffer,
				pControlTransfer->TransferBufferMDL,
				pControlTransfer->TransferBufferLength, TRUE);
			if(((!bReadFromDevice) && (!bReturnedFromHCD)) || (bReadFromDevice && bReturnedFromHCD))
			{
				DumpTransferBuffer(b,(PUCHAR)pControlTransfer->TransferBuffer,
					pControlTransfer->TransferBufferMDL, pControlTransfer->TransferBufferLength, FALSE);
			}

			BufferPrintf(b,"  UrbLink              = %x\n", pControlTransfer->UrbLink);
			BufferPrintf(b,"  SetupPacket          =\n");
			DumpBuffer(b,pControlTransfer->SetupPacket,
				sizeof(pControlTransfer->SetupPacket));

			if(pControlTransfer->UrbLink)
			{
				BufferPrintf(b,"---> Linked URB:\n");
				DumpURB(b,pControlTransfer->UrbLink, bReturnedFromHCD);
				BufferPrintf(b,"---< Linked URB\n");
			}
		}
		break;

	case URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER: // tested [22/11/2001]
		{
			struct _URB_BULK_OR_INTERRUPT_TRANSFER *pBulkOrInterruptTransfer = (struct _URB_BULK_OR_INTERRUPT_TRANSFER *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER:\n");
			if(pBulkOrInterruptTransfer->Hdr.Length < sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER ))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n",
					pBulkOrInterruptTransfer->Hdr.Length,
					sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));
				return ;
			}

			BOOLEAN bReadFromDevice = (BOOLEAN)(pBulkOrInterruptTransfer->TransferFlags & USBD_TRANSFER_DIRECTION_IN);
			DumpPipeHandle(b,"  PipeHandle          ",pBulkOrInterruptTransfer->PipeHandle);
			BufferPrintf(b,"  TransferFlags        = %x (%s, %sUSBD_SHORT_TRANSFER_OK)\n", pBulkOrInterruptTransfer->TransferFlags,
				bReadFromDevice ? "USBD_TRANSFER_DIRECTION_IN" : "USBD_TRANSFER_DIRECTION_OUT",
				pBulkOrInterruptTransfer->TransferFlags & USBD_SHORT_TRANSFER_OK ? "":"~");
			DumpTransferBuffer(b,(PUCHAR)pBulkOrInterruptTransfer->TransferBuffer, pBulkOrInterruptTransfer->TransferBufferMDL, pBulkOrInterruptTransfer->TransferBufferLength, TRUE);
			if(((!bReadFromDevice) && (!bReturnedFromHCD)) || (bReadFromDevice && bReturnedFromHCD))
			{
				DumpTransferBuffer(b,(PUCHAR)pBulkOrInterruptTransfer->TransferBuffer, pBulkOrInterruptTransfer->TransferBufferMDL, pBulkOrInterruptTransfer->TransferBufferLength, FALSE);
			}

			BufferPrintf(b,"  UrbLink              = %x\n", pBulkOrInterruptTransfer->UrbLink);
			if(pBulkOrInterruptTransfer->UrbLink)
			{
				BufferPrintf(b,"---> Linked URB:\n");
				DumpURB(b,pBulkOrInterruptTransfer->UrbLink, bReturnedFromHCD);
				BufferPrintf(b,"---< Linked URB\n");
			}
		}
		break;

	case URB_FUNCTION_ISOCH_TRANSFER: // tested [22/11/2001]
		{
			struct _URB_ISOCH_TRANSFER *pIsochTransfer = (struct _URB_ISOCH_TRANSFER *) pUrb;
			BOOLEAN dumpBuffer = FALSE;

			BufferPrintf(b,"-- URB_FUNCTION_ISOCH_TRANSFER:\n");
			if(pIsochTransfer->Hdr.Length < sizeof(struct _URB_ISOCH_TRANSFER ))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n",
					pIsochTransfer->Hdr.Length, sizeof(struct _URB_ISOCH_TRANSFER));
				return ;
			}

			BOOLEAN bReadFromDevice = (BOOLEAN)(pIsochTransfer->TransferFlags & USBD_TRANSFER_DIRECTION_IN);
			DumpPipeHandle(b,"  PipeHandle          ",pIsochTransfer->PipeHandle);
			BufferPrintf(b,"  TransferFlags        = %x (%s, %sUSBD_SHORT_TRANSFER_OK%s\n", pIsochTransfer->TransferFlags,
				bReadFromDevice ? "USBD_TRANSFER_DIRECTION_IN" : "USBD_TRANSFER_DIRECTION_OUT",
				pIsochTransfer->TransferFlags & USBD_SHORT_TRANSFER_OK ? "":"~",
				pIsochTransfer->TransferFlags & USBD_START_ISO_TRANSFER_ASAP ? ", USBD_START_ISO_TRANSFER_ASAP" : "");

			DumpTransferBuffer(b,(PUCHAR)pIsochTransfer->TransferBuffer, pIsochTransfer->TransferBufferMDL, pIsochTransfer->TransferBufferLength, TRUE);
			if( (!bReadFromDevice && !bReturnedFromHCD)
				|| (bReadFromDevice && bReturnedFromHCD) )
			{
				dumpBuffer = TRUE;
//				DumpTransferBuffer((PUCHAR)pIsochTransfer->TransferBuffer, pIsochTransfer->TransferBufferMDL, pIsochTransfer->TransferBufferLength, FALSE);
			}

			BufferPrintf(b,"  StartFrame           = %x\n", pIsochTransfer->StartFrame);
			BufferPrintf(b,"  NumberOfPackets      = %x\n", pIsochTransfer->NumberOfPackets);
			if(bReturnedFromHCD)
				BufferPrintf(b,"  ErrorCount           = %x\n", pIsochTransfer->ErrorCount);

			for(ULONG p=0; p < pIsochTransfer->NumberOfPackets; p++)
			{
				BufferPrintf(b,"  IsoPacket[%d].Offset = %d\n",p, pIsochTransfer->IsoPacket[p].Offset);
				BufferPrintf(b,"  IsoPacket[%d].Length = %d\n",p, pIsochTransfer->IsoPacket[p].Length);
				// possible value for Status are described in <usbdi.h>
				// search for USBD_STATUS_SUCCESS (0).
				if(bReturnedFromHCD)
					BufferPrintf(b,"  IsoPacket[%d].Status = %x\n",p, pIsochTransfer->IsoPacket[p].Status);
				if (dumpBuffer)
					DumpTransferBuffer(b,(PUCHAR)pIsochTransfer->TransferBuffer,
						pIsochTransfer->TransferBufferMDL,
						pIsochTransfer->IsoPacket[p].Length, FALSE,
						pIsochTransfer->IsoPacket[p].Offset);
			}

			BufferPrintf(b,"  UrbLink              = %x\n", pIsochTransfer->UrbLink);
			if(pIsochTransfer->UrbLink)
			{
				BufferPrintf(b,"---> Linked URB:\n");
				DumpURB(b,pIsochTransfer->UrbLink, bReturnedFromHCD);
				BufferPrintf(b,"---< Linked URB\n");
			}
		}
		break;

	case URB_FUNCTION_RESET_PIPE:
		{
			struct _URB_PIPE_REQUEST   *pResetPipe = (struct _URB_PIPE_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_RESET_PIPE:\n");
			if(pResetPipe->Hdr.Length < sizeof(struct _URB_PIPE_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pResetPipe->Hdr.Length, sizeof(struct _URB_PIPE_REQUEST));
				return ;
			}

			if(!bReturnedFromHCD)
				DumpPipeHandle(b,"  PipeHandle",pResetPipe->PipeHandle);
		}
		break;

	case URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE:
		{
			struct _URB_CONTROL_DESCRIPTOR_REQUEST   *pGetDescriptorFromDevice = (struct _URB_CONTROL_DESCRIPTOR_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE:\n");
			if(pGetDescriptorFromDevice->Hdr.Length < sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pGetDescriptorFromDevice->Hdr.Length, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));
				return ;
			}

			DumpDescriptorRequest(b,pGetDescriptorFromDevice, TRUE, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT:
		{
			struct _URB_CONTROL_DESCRIPTOR_REQUEST   *pGetDescriptorFromEndpoint = (struct _URB_CONTROL_DESCRIPTOR_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT:\n");
			if(pGetDescriptorFromEndpoint->Hdr.Length < sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pGetDescriptorFromEndpoint->Hdr.Length, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));
				return ;
			}

			DumpDescriptorRequest(b,pGetDescriptorFromEndpoint, TRUE, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE:
		{
			struct _URB_CONTROL_DESCRIPTOR_REQUEST   *pGetDescriptorFromInterface = (struct _URB_CONTROL_DESCRIPTOR_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE:\n");
			if(pGetDescriptorFromInterface->Hdr.Length < sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pGetDescriptorFromInterface->Hdr.Length, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));
				return ;
			}

			DumpDescriptorRequest(b,pGetDescriptorFromInterface, TRUE, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE:
		{
			struct _URB_CONTROL_DESCRIPTOR_REQUEST   *pSetDescriptorToDevice = (struct _URB_CONTROL_DESCRIPTOR_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE:\n");
			if(pSetDescriptorToDevice->Hdr.Length < sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pSetDescriptorToDevice->Hdr.Length, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));
				return ;
			}

			DumpDescriptorRequest(b,pSetDescriptorToDevice, FALSE, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT:
		{
			struct _URB_CONTROL_DESCRIPTOR_REQUEST   *pSetDescriptorToEndpoint = (struct _URB_CONTROL_DESCRIPTOR_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT:\n");
			if(pSetDescriptorToEndpoint->Hdr.Length < sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pSetDescriptorToEndpoint->Hdr.Length, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));
				return ;
			}

			DumpDescriptorRequest(b,pSetDescriptorToEndpoint, TRUE, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE:
		{
			struct _URB_CONTROL_DESCRIPTOR_REQUEST   *pSetDescriptorToInterface = (struct _URB_CONTROL_DESCRIPTOR_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE:\n");
			if(pSetDescriptorToInterface->Hdr.Length < sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pSetDescriptorToInterface->Hdr.Length, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));				return ;
			}

			DumpDescriptorRequest(b,pSetDescriptorToInterface, TRUE, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_SET_FEATURE_TO_DEVICE:
		{
			struct _URB_CONTROL_FEATURE_REQUEST   *pSetFeatureToDevice = (struct _URB_CONTROL_FEATURE_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_SET_FEATURE_TO_DEVICE:\n");
			if(pSetFeatureToDevice->Hdr.Length < sizeof(struct _URB_CONTROL_FEATURE_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pSetFeatureToDevice->Hdr.Length, sizeof(struct _URB_CONTROL_FEATURE_REQUEST));
				return ;
			}

			DumpFeatureRequest(b,pSetFeatureToDevice, TRUE, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_SET_FEATURE_TO_INTERFACE:
		{
			struct _URB_CONTROL_FEATURE_REQUEST   *pSetFeatureToInterface = (struct _URB_CONTROL_FEATURE_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_SET_FEATURE_TO_INTERFACE:\n");
			if(pSetFeatureToInterface->Hdr.Length < sizeof(struct _URB_CONTROL_FEATURE_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pSetFeatureToInterface->Hdr.Length, sizeof(struct _URB_CONTROL_FEATURE_REQUEST));
				return ;
			}

			DumpFeatureRequest(b,pSetFeatureToInterface, TRUE, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_SET_FEATURE_TO_ENDPOINT:
		{
			struct _URB_CONTROL_FEATURE_REQUEST   *pSetFeatureToEndpoint = (struct _URB_CONTROL_FEATURE_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_SET_FEATURE_TO_ENDPOINT:\n");
			if(pSetFeatureToEndpoint->Hdr.Length < sizeof(struct _URB_CONTROL_FEATURE_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pSetFeatureToEndpoint->Hdr.Length, sizeof(struct _URB_CONTROL_FEATURE_REQUEST));
				return ;
			}

			DumpFeatureRequest(b,pSetFeatureToEndpoint, TRUE, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_SET_FEATURE_TO_OTHER:
		{
			struct _URB_CONTROL_FEATURE_REQUEST   *pSetFeatureToOther = (struct _URB_CONTROL_FEATURE_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_SET_FEATURE_TO_OTHER:\n");
			if(pSetFeatureToOther->Hdr.Length < sizeof(struct _URB_CONTROL_FEATURE_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pSetFeatureToOther->Hdr.Length, sizeof(struct _URB_CONTROL_FEATURE_REQUEST));
				return ;
			}

			DumpFeatureRequest(b,pSetFeatureToOther, TRUE, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE:
		{
			struct _URB_CONTROL_FEATURE_REQUEST   *pClearFeatureToDevice = (struct _URB_CONTROL_FEATURE_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE:\n");
			if(pClearFeatureToDevice->Hdr.Length < sizeof(struct _URB_CONTROL_FEATURE_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pClearFeatureToDevice->Hdr.Length, sizeof(struct _URB_CONTROL_FEATURE_REQUEST));
				return ;
			}

			DumpFeatureRequest(b,pClearFeatureToDevice, TRUE, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE:
		{
			struct _URB_CONTROL_FEATURE_REQUEST   *pClearFeatureToInterface = (struct _URB_CONTROL_FEATURE_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE:\n");
			if(pClearFeatureToInterface->Hdr.Length < sizeof(struct _URB_CONTROL_FEATURE_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pClearFeatureToInterface->Hdr.Length, sizeof(struct _URB_CONTROL_FEATURE_REQUEST));
				return ;
			}

			DumpFeatureRequest(b,pClearFeatureToInterface, TRUE, bReturnedFromHCD);

		}
		break;

	case URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT:
		{
			struct _URB_CONTROL_FEATURE_REQUEST   *pClearFeatureToEndpoint = (struct _URB_CONTROL_FEATURE_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT:\n");
			if(pClearFeatureToEndpoint->Hdr.Length < sizeof(struct _URB_CONTROL_FEATURE_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pClearFeatureToEndpoint->Hdr.Length, sizeof(struct _URB_CONTROL_FEATURE_REQUEST));
				return ;
			}

			DumpFeatureRequest(b,pClearFeatureToEndpoint, TRUE, bReturnedFromHCD);

		}
		break;

	case URB_FUNCTION_CLEAR_FEATURE_TO_OTHER:
		{
			struct _URB_CONTROL_FEATURE_REQUEST   *pClearFeatureToOther = (struct _URB_CONTROL_FEATURE_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_CLEAR_FEATURE_TO_OTHER:\n");
			if(pClearFeatureToOther->Hdr.Length < sizeof(struct _URB_CONTROL_FEATURE_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pClearFeatureToOther->Hdr.Length, sizeof(struct _URB_CONTROL_FEATURE_REQUEST));
				return ;
			}

			DumpFeatureRequest(b,pClearFeatureToOther, TRUE, bReturnedFromHCD);

		}
		break;

	case URB_FUNCTION_GET_STATUS_FROM_DEVICE:
		{
			struct _URB_CONTROL_GET_STATUS_REQUEST *pGetStatusFromDevice = (struct _URB_CONTROL_GET_STATUS_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_GET_STATUS_FROM_DEVICE:\n");
			if(pGetStatusFromDevice->Hdr.Length < sizeof(struct _URB_CONTROL_GET_STATUS_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pGetStatusFromDevice->Hdr.Length, sizeof(struct _URB_CONTROL_GET_STATUS_REQUEST));
				return ;
			}

			DumpGetStatusRequest(b,pGetStatusFromDevice, bReturnedFromHCD);

		}

	case URB_FUNCTION_GET_STATUS_FROM_INTERFACE:
		return ;
		{
			struct _URB_CONTROL_GET_STATUS_REQUEST *pGetStatusFromInterface = (struct _URB_CONTROL_GET_STATUS_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_GET_STATUS_FROM_INTERFACE:\n");
			if(pGetStatusFromInterface->Hdr.Length < sizeof(struct _URB_CONTROL_GET_STATUS_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pGetStatusFromInterface->Hdr.Length, sizeof(struct _URB_CONTROL_GET_STATUS_REQUEST));
				return ;
			}

			DumpGetStatusRequest(b,pGetStatusFromInterface, bReturnedFromHCD);

		}

	case URB_FUNCTION_GET_STATUS_FROM_ENDPOINT:
		{
			struct _URB_CONTROL_GET_STATUS_REQUEST *pGetStatusFromEndpoint = (struct _URB_CONTROL_GET_STATUS_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_GET_STATUS_FROM_ENDPOINT:\n");
			if(pGetStatusFromEndpoint->Hdr.Length < sizeof(struct _URB_CONTROL_GET_STATUS_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pGetStatusFromEndpoint->Hdr.Length, sizeof(struct _URB_CONTROL_GET_STATUS_REQUEST));
				return ;
			}

			DumpGetStatusRequest(b,pGetStatusFromEndpoint, bReturnedFromHCD);

		}
		break;

	case URB_FUNCTION_GET_STATUS_FROM_OTHER:
		{
			struct _URB_CONTROL_GET_STATUS_REQUEST *pGetStatusFromOther = (struct _URB_CONTROL_GET_STATUS_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_GET_STATUS_FROM_OTHER:\n");
			if(pGetStatusFromOther->Hdr.Length < sizeof(struct _URB_CONTROL_GET_STATUS_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pGetStatusFromOther->Hdr.Length, sizeof(struct _URB_CONTROL_GET_STATUS_REQUEST));
				return ;
			}

			DumpGetStatusRequest(b,pGetStatusFromOther, bReturnedFromHCD);

		}
		break;

	case URB_FUNCTION_VENDOR_DEVICE:
		{
			struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *pFunctionVendorDevice = (struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_VENDOR_DEVICE:\n");
			if(pFunctionVendorDevice->Hdr.Length < sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pFunctionVendorDevice->Hdr.Length, sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));
				return ;
			}

			DumpVendorOrClassRequest(b,pFunctionVendorDevice, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_VENDOR_INTERFACE:
		{
			struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *pFunctionVendorInterface = (struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_VENDOR_INTERFACE:\n");
			if(pFunctionVendorInterface->Hdr.Length < sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pFunctionVendorInterface->Hdr.Length, sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));
				return ;
			}

			DumpVendorOrClassRequest(b,pFunctionVendorInterface, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_VENDOR_ENDPOINT:
		{
			struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *pFunctionVendorEndpoint = (struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_VENDOR_ENDPOINT:\n");
			if(pFunctionVendorEndpoint->Hdr.Length < sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n",
					pFunctionVendorEndpoint->Hdr.Length,
					sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));
				return ;
			}

			DumpVendorOrClassRequest(b,pFunctionVendorEndpoint, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_VENDOR_OTHER:
		{
			struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *pFunctionVendorOther = (struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_VENDOR_OTHER:\n");
			if(pFunctionVendorOther->Hdr.Length < sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n",
					pFunctionVendorOther->Hdr.Length,
					sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));
				return ;
			}

			DumpVendorOrClassRequest(b,pFunctionVendorOther, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_CLASS_DEVICE:
		{
			struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *pFunctionClassDevice = (struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_CLASS_DEVICE:\n");
			if(pFunctionClassDevice->Hdr.Length < sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n",
					pFunctionClassDevice->Hdr.Length,
					sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));
				return ;
			}

			DumpVendorOrClassRequest(b,pFunctionClassDevice, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_CLASS_INTERFACE:
		{
			struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *pFunctionClassInterface = (struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_CLASS_INTERFACE:\n");
			if(pFunctionClassInterface->Hdr.Length < sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST))
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n", pFunctionClassInterface->Hdr.Length, sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));

			DumpVendorOrClassRequest(b,pFunctionClassInterface, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_CLASS_ENDPOINT:
		{
			struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *pFunctionClassEndpoint = (struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_CLASS_ENDPOINT:\n");
			if(pFunctionClassEndpoint->Hdr.Length < sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n",
					pFunctionClassEndpoint->Hdr.Length,
					sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));
				return ;
			}

			DumpVendorOrClassRequest(b,pFunctionClassEndpoint, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_CLASS_OTHER:
		{
			struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *pFunctionClassOther = (struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_CLASS_OTHER:\n");
			if(pFunctionClassOther->Hdr.Length < sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n",
					pFunctionClassOther->Hdr.Length,
					sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));
				return ;
			}

			DumpVendorOrClassRequest(b,pFunctionClassOther, bReturnedFromHCD);
		}
		break;

	case URB_FUNCTION_GET_CONFIGURATION:
		{
			struct _URB_CONTROL_GET_CONFIGURATION_REQUEST *pGetConfiguration = (struct _URB_CONTROL_GET_CONFIGURATION_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_GET_CONFIGURATION:\n");
			if(pGetConfiguration->Hdr.Length < sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n",
					pGetConfiguration->Hdr.Length,
					sizeof(struct _URB_CONTROL_GET_CONFIGURATION_REQUEST));
				return ;
			}

			DumpTransferBuffer(b,(PUCHAR)pGetConfiguration->TransferBuffer, pGetConfiguration->TransferBufferMDL, pGetConfiguration->TransferBufferLength, TRUE);
			if(pGetConfiguration->TransferBufferLength != 1)
				BufferPrintf(b,"  *** error - TransferBufferLength should be 1!\n");

			if(bReturnedFromHCD)
			{
				DumpTransferBuffer(b,(PUCHAR)pGetConfiguration->TransferBuffer, pGetConfiguration->TransferBufferMDL, pGetConfiguration->TransferBufferLength, FALSE);
			}

			BufferPrintf(b,"  UrbLink              = %x\n", pGetConfiguration->UrbLink);
			if(pGetConfiguration->UrbLink)
			{
				BufferPrintf(b,"---> Linked URB:\n");
				DumpURB(b,pGetConfiguration->UrbLink, bReturnedFromHCD);
				BufferPrintf(b,"---< Linked URB\n");
			}
		}
		break;

	case URB_FUNCTION_GET_INTERFACE:
		{
			struct _URB_CONTROL_GET_INTERFACE_REQUEST *pGetInterface = (struct _URB_CONTROL_GET_INTERFACE_REQUEST *) pUrb;

			BufferPrintf(b,"-- URB_FUNCTION_GET_INTERFACE:\n");
			if(pGetInterface->Hdr.Length < sizeof(struct _URB_CONTROL_GET_INTERFACE_REQUEST))
			{
				BufferPrintf(b,"!!! Hdr.Length is wrong! (is: %d, should be at least: %d)\n",
					pGetInterface->Hdr.Length, sizeof(struct _URB_CONTROL_GET_CONFIGURATION_REQUEST));
				return ;
			}

			DumpTransferBuffer(b,(PUCHAR)pGetInterface->TransferBuffer, pGetInterface->TransferBufferMDL, pGetInterface->TransferBufferLength, TRUE);
			if(pGetInterface->TransferBufferLength != 1)
				BufferPrintf(b,"  *** error - TransferBufferLength should be 1!\n");

			if(bReturnedFromHCD)
			{
				DumpTransferBuffer(b,(PUCHAR)pGetInterface->TransferBuffer, pGetInterface->TransferBufferMDL, pGetInterface->TransferBufferLength, FALSE);
			}

			BufferPrintf(b,"  Interface            = %x\n", pGetInterface->UrbLink);
			BufferPrintf(b,"  UrbLink              = %x\n", pGetInterface->UrbLink);
			if(pGetInterface->UrbLink)
			{
				BufferPrintf(b,"---> Linked URB:\n");
				DumpURB(b,pGetInterface->UrbLink, bReturnedFromHCD);
				BufferPrintf(b,"---< Linked URB\n");
			}
		}
		break;

	default:
		BufferPrintf(b,"******* non printable URB with function code 0x%x ********\n", wFunction);
		break;
	}	// end of mega switch
}

///////////////////////////////////////////////////////////////////////////////
