import sys

def pci_class_decode(pci_class):
    print('Revision ID: {}'.format(hex(pci_class & 0xFF)))
    print('Program IF:  {}'.format(hex((pci_class >> 8) & 0xFF)))
    print('Subclass:    {}'.format(hex((pci_class >> 16) & 0xFF)))
    print('Class:       {}'.format(hex((pci_class >> 24) & 0xFF)))

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('usage: {} [PCI Device Class] ...'.format(sys.argv[0]))
        sys.exit(0)
    
    for i in range(1, len(sys.argv)):
        pci_class_decode(int(sys.argv[i], 0))
        print('-'*15)
