% ��MATLAB��CCS���ӣ��� eeprom2FcAutoGenerate[] ��ȡ������
% д���ļ� f_table_eeprom2Fc_autoGenerate.c �С�

% ����1�н���ִ��1��
% [boardnum,procnum] = boardprocsel; cc=ticcs('boardnum',boardnum,'procnum',procnum);

% ��ȡ eeprom2Fc[] �ĳ��ȡ�
sizeOfTable_eeprom2Fc = read(cc,address(cc,'sizeOfTable_eeprom2Fc'),'int16')
sizeOfTable = sizeOfTable_eeprom2Fc;
i = 0:sizeOfTable-1;

% ��ȡ eeprom2Fc[]
eeprom2FcAutoGenerate = read(cc,address(cc,'eeprom2FcAutoGenerate'),'int16', sizeOfTable);

maxEepromAddr = sizeOfTable_eeprom2Fc-1;


% д�뵽�ļ���
fid = fopen('f_table_eeprom2Fc_autoGenerate.c', 'w+');
fprintf(fid, '// DSP�����Զ��������飬matlab��ȡCCS�е�RAM���ݣ��Զ�д���ļ���\n');

fprintf(fid, '// \n');
fprintf(fid, '// ��Ӧ��ϵ�� \n');
fprintf(fid, '// y = eeprom2Fc[i] \n');
fprintf(fid, '// i, �����±꣬----�ù�������EEPROM��λ�� \n');
fprintf(fid, '// y, �����ֵ��----���������� \n');
fprintf(fid, '// ������������    %d \n', sizeOfTable_eeprom2Fc);
fprintf(fid, '// ���EEPROM��ַ��%d \n', maxEepromAddr);

fprintf(fid, '// \n');
fprintf(fid, 'const Uint16 eeprom2Fc[] =\n');
fprintf(fid, '{\n');
fprintf(fid, '//  y       // i\n');
fprintf(fid, '// \n');
fprintf(fid, '    %-4d,   // %d\n', [eeprom2FcAutoGenerate; i]);
fprintf(fid, '};\n\n');
fclose(fid);



