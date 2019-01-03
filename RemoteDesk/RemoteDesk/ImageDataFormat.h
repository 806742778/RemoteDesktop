#pragma once

const size_t MAX_IAMGE_DATA_SIZE = 548;
const size_t MAX_IMG_BUFFER_SIZE = 1024 * 1024;

// ���ݰ� = ���λ + ��ͷ + ͼ������
typedef struct tagImagePackageHeader
{
	unsigned int nMarkSize;		// ���λ��С
	unsigned int nHeaderSize;	// ��ͷ��С
	unsigned int nDataSize;		// �˰�ͼ�����ݵĴ�С
	unsigned int nImageSize;	// ��ͼƬ�ܴ�С
	unsigned int nPackageNum;	// һ���ֳ��˶��ٸ�С��
	unsigned int nPackageIndex;	// ��ǰ���ǵڼ���С��
	unsigned int nDataOffset;	// ��ǰͼ��������ͼ���е�ƫ����
	unsigned int nImageId;		// ͼƬID
}ImagePackageHeader;
