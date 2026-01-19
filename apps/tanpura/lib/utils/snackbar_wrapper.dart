import 'package:flutter/material.dart';

/// A utility function to show a Snackbar with a given message.
///
/// This wrapper simplifies showing Snackbars throughout the app by requiring only
/// the BuildContext and the message. Additional customizations can be added as needed.
void showSnackBar(
  BuildContext context,
  String message, {
  Duration duration = const Duration(seconds: 4),
}) {
  ScaffoldMessenger.of(
    context,
  ).showSnackBar(SnackBar(content: Text(message), duration: duration));
}
