import 'dart:io';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:path_provider/path_provider.dart';
import 'package:tanpura_engine/tanpura_engine.dart';
import 'package:share_plus/share_plus.dart';

class ExportButton extends StatefulWidget {
  const ExportButton({super.key});

  @override
  State<ExportButton> createState() => _ExportButtonState();
}

class _ExportButtonState extends State<ExportButton> {
  bool _exporting = false;
  String? _lastExportPath;

  Future<void> _exportToLocal() async {
    setState(() => _exporting = true);

    try {
      // Use external storage for emulator (easier to access via adb)
      Directory? directory;
      if (Platform.isAndroid) {
        directory = await getExternalStorageDirectory();
      }
      directory ??= await getApplicationDocumentsDirectory();

      final timestamp = DateTime.now().millisecondsSinceEpoch;
      final filePath = '${directory.path}/tanpura_$timestamp.wav';

      // Export 10 seconds of audio
      final success =
          await TanpuraEngine.exportWav(filePath, durationSec: 10.0);

      if (success && mounted) {
        setState(() => _lastExportPath = filePath);
        _showExportSuccessDialog(filePath);
      } else if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(content: Text('Export failed')),
        );
      }
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Error: $e')),
        );
      }
    } finally {
      if (mounted) {
        setState(() => _exporting = false);
      }
    }
  }

  void _showExportSuccessDialog(String filePath) {
    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        backgroundColor: const Color(0xFF1A1A1A),
        title: const Text(
          'Export Successful',
          style: TextStyle(color: Color(0xFFC9A24D)),
        ),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              'File saved to:',
              style: TextStyle(color: Colors.white70),
            ),
            const SizedBox(height: 8),
            Container(
              padding: const EdgeInsets.all(8),
              decoration: BoxDecoration(
                color: Colors.black26,
                borderRadius: BorderRadius.circular(4),
              ),
              child: SelectableText(
                filePath,
                style: const TextStyle(
                  color: Colors.white,
                  fontSize: 12,
                  fontFamily: 'monospace',
                ),
              ),
            ),
            const SizedBox(height: 16),
            const Text(
              'To pull from emulator:',
              style: TextStyle(color: Colors.white70, fontSize: 12),
            ),
            const SizedBox(height: 4),
            Container(
              padding: const EdgeInsets.all(8),
              decoration: BoxDecoration(
                color: Colors.black26,
                borderRadius: BorderRadius.circular(4),
              ),
              child: SelectableText(
                'adb pull "$filePath" .',
                style: const TextStyle(
                  color: Colors.green,
                  fontSize: 11,
                  fontFamily: 'monospace',
                ),
              ),
            ),
          ],
        ),
        actions: [
          TextButton(
            onPressed: () {
              Clipboard.setData(ClipboardData(text: filePath));
              ScaffoldMessenger.of(context).showSnackBar(
                const SnackBar(content: Text('Path copied to clipboard')),
              );
            },
            child: const Text('Copy Path'),
          ),
          TextButton(
            onPressed: () async {
              Navigator.pop(context);
              // Try to share
              await Share.shareXFiles(
                [XFile(filePath)],
                text: 'Tanpura audio sample',
              );
            },
            child: const Text('Share'),
          ),
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: const Text('Close'),
          ),
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      children: [
        ElevatedButton.icon(
          onPressed: _exporting ? null : _exportToLocal,
          icon: _exporting
              ? const SizedBox(
                  width: 16,
                  height: 16,
                  child: CircularProgressIndicator(strokeWidth: 2),
                )
              : const Icon(Icons.save),
          label: Text(_exporting ? 'Exporting...' : 'Export Audio (10 sec)'),
          style: ElevatedButton.styleFrom(
            backgroundColor: const Color(0xFFC9A24D),
            foregroundColor: Colors.black,
            padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 12),
          ),
        ),
        if (_lastExportPath != null) ...[
          const SizedBox(height: 8),
          Text(
            'Last export: ${_lastExportPath!.split('/').last}',
            style: const TextStyle(color: Colors.white38, fontSize: 11),
          ),
        ],
      ],
    );
  }
}
